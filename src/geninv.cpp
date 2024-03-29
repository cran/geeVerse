#include <Rcpp.h>
#include <RcppEigen.h>
#include <Eigen/LU>
#include <Eigen/SparseCholesky>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cmath>

// via the depends attribute we tell Rcpp to create hooks for
// RcppEigen so that the build process will know what to do
//
// [[Rcpp::depends(RcppEigen)]]

using namespace Rcpp;
using namespace RcppEigen;

// [[Rcpp::export]]
SEXP geninv(SEXP GG){
  try {
    using Eigen::Map;
    using Eigen::MatrixXd;
    using Eigen::Lower;
    const Eigen::Map<MatrixXd> G(as<Map<MatrixXd> >(GG));
    const int n(G.rows());
    const int m(G.cols());
    const int mn(std::min(n, m));

    bool transp(false);
    double tol(1.0e-10);
    MatrixXd A(MatrixXd(mn, mn));
    MatrixXd L(MatrixXd(mn, mn).setZero());



    if (n < m) {
      transp = true;
      A = G * G.transpose();
    } else {
      A = G.transpose() * G;
    }

    int r = 0;
    for (int k = 0; k < mn; k++) {
      r++;

      if (r == 1) {
        L.block(k, r - 1, mn - k, 1) = A.block(k, k, mn - k, 1);
      } else {
        L.block(k, r - 1, mn - k, 1) = A.block(k, k, mn - k, 1) -
                L.block(k, 0, mn - k, r - 1) * L.block(k, 0, 1, r - 1).adjoint();
      }

      if (L(k, r - 1) > tol) {
        L.block(k, r - 1, 1, 1) = L.block(k, r - 1, 1, 1).array().sqrt();
        if (k + 1 < mn) {
          L.block(k + 1, r - 1, mn - k - 1, 1) = L.block(k + 1, r - 1, mn - k - 1, 1) / L(k, r - 1);
        }
      } else {
        r--;
      }
    }

    MatrixXd M(MatrixXd(r, r));
    M = L.block(0, 0, mn, r);
    M = (M.transpose() * M).inverse();

    MatrixXd Y(MatrixXd(m, n));

    if (transp) {
      Y = G.adjoint() * L.block(0, 0, mn, r) * M * M * L.block(0, 0, mn, r).adjoint();
    } else {
      Y = L.block(0, 0, mn, r) * M * M * L.block(0, 0, mn, r).adjoint() * G.adjoint();
    }

    return wrap(Y);
  } catch (std::exception &ex) {
    forward_exception_to_r(ex);
  } catch (...) {
    ::Rf_error("C++ exception (unknown reason)");
  }
  return R_NilValue; //-Wall
}




