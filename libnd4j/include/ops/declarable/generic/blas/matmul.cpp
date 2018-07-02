//
// @author raver119@gmail.com, created on 07.10.2017.
// @author GS <sgazeos@gmail.com>, modified
// @author Yurii Shyrma (iuriish@yahoo.com), fully rewritten
//

#include <op_boilerplate.h> 
#if NOT_EXCLUDED(OP_matmul)

#include <ops/declarable/CustomOperations.h>
#include <ops/declarable/helpers/matmul.h>

namespace nd4j {
namespace ops  {

CUSTOM_OP_IMPL(matmul, 2, 1, false, 0, -2) {
    
    NDArray<T> *x = INPUT_VARIABLE(0);
    NDArray<T> *y = INPUT_VARIABLE(1);
    NDArray<T> *z = OUTPUT_VARIABLE(0);    

    const int iSize  = (int) block.getIArguments()->size();   
          int transA = iSize > 0 ? INT_ARG(0) : 0;
          int transB = iSize > 1 ? INT_ARG(1) : 0;
    const int transC = iSize > 2 ? INT_ARG(2) : 0;

    const int xRank = x->rankOf();
    const int yRank = y->rankOf();
    const int zRank = z->rankOf();

    // ******* input validation ******* //
    REQUIRE_TRUE(xRank > 1 && yRank > 1 && zRank > 1, 0, "MATMUL OP: input and output arrays must have rank bigger than 1, but got instead: x rank = %i, y rank = %i, z rank = %i !", xRank, yRank, zRank);
    REQUIRE_TRUE(xRank == yRank && yRank == zRank, 0, "MATMUL OP: input and output arrays must have the same rank, but got instead: x rank = %i, y rank = %i, z rank = %i !", xRank, yRank, zRank);

    if(transC) {
        x = INPUT_VARIABLE(1);
        y = INPUT_VARIABLE(0);
        transA = !transA;
        transB = !transB;
    }

    const int xLastDim       = transA ? -2 : -1; 
    const int yLastDim       = transB ? -2 : -1;
    const int xLastButOneDim = transA ? -1 : -2; 
    const int yLastButOneDim = transB ? -1 : -2;
    
    REQUIRE_TRUE(x->sizeAt(xLastDim) == y->sizeAt(yLastButOneDim) && x->sizeAt(xLastButOneDim) == z->sizeAt(-2) && y->sizeAt(yLastDim) == z->sizeAt(-1), 0, "MATMUL OP: input/output arrays have inconsistent shapes for matrix product: x %s, y %s, z %s !", ShapeUtils<T>::shapeAsString(x).c_str(), ShapeUtils<T>::shapeAsString(y).c_str(), ShapeUtils<T>::shapeAsString(z).c_str());
    
    if(xRank > 2)   // outer dims must be the same
        for(int i = 0; i < xRank-2; ++i)
            REQUIRE_TRUE(x->sizeAt(i) == y->sizeAt(i) && y->sizeAt(i) == z->sizeAt(i), 0, "MATMUL OP: input/output arrays have inconsistent shapes for matrix product: x %s, y %s, z %s !", ShapeUtils<T>::shapeAsString(x).c_str(), ShapeUtils<T>::shapeAsString(y).c_str(), ShapeUtils<T>::shapeAsString(z).c_str());
    // ******* end of input validation ******* //

    NDArrayFactory<T>::tensorDot(x, y, z, {xLastDim}, {yLastButOneDim});

    return Status::OK();
}
DECLARE_SYN(mMul, matmul);
DECLARE_SYN(mmul, matmul);
DECLARE_SYN(gemm, matmul);
DECLARE_SYN(gemv, matmul);
DECLARE_SYN(dot, matmul);


DECLARE_SHAPE_FN(matmul) {
    
    Nd4jLong* xShapeInfo = inputShape->at(0);
    Nd4jLong* yShapeInfo = inputShape->at(1);    

    const int iSize  = (int) block.getIArguments()->size();   
          int transA = iSize > 0 ? INT_ARG(0) : 0;
          int transB = iSize > 1 ? INT_ARG(1) : 0;
    const int transC = iSize > 2 ? INT_ARG(2) : 0;

    const int xRank = xShapeInfo[0];
    const int yRank = yShapeInfo[0];

    // ******* input validation ******* //
    REQUIRE_TRUE(xRank > 1 && yRank > 1, 0, "MATMUL OP: input arrays must have rank bigger than 1, but got instead: x rank = %i, y rank = %i !", xRank, yRank);
    REQUIRE_TRUE(xRank == yRank, 0, "MATMUL OP: input arrays must have the same rank, but got instead: x rank = %i, y rank = %i !", xRank, yRank);

    if(transC) {
        xShapeInfo = inputShape->at(1);
        yShapeInfo = inputShape->at(0);
        transA = !transA;
        transB = !transB;
    }

    const int xLastDim       = transA ? -2 : -1; 
    const int yLastDim       = transB ? -2 : -1;
    const int yLastButOneDim = transB ? -1 : -2;     
    
    REQUIRE_TRUE(shape::sizeAt(xShapeInfo, xLastDim) == shape::sizeAt(yShapeInfo, yLastButOneDim), 0, "MATMUL OP: input arrays have inconsistent shapes for matrix product: x %s, y %s !", ShapeUtils<T>::shapeAsString(xShapeInfo).c_str(), ShapeUtils<T>::shapeAsString(yShapeInfo).c_str());
    
    if(xRank > 2)   // outer dims must be the same
        for(int i = 0; i < xRank-2; ++i)
            REQUIRE_TRUE(shape::sizeAt(xShapeInfo, i) == shape::sizeAt(yShapeInfo, i), 0, "MATMUL OP: input arrays have inconsistent shapes for matrix product: x %s, y %s !", ShapeUtils<T>::shapeAsString(xShapeInfo).c_str(), ShapeUtils<T>::shapeAsString(yShapeInfo).c_str());
    // ******* end of input validation ******* //

    std::vector<int> permutAt, permutBt;
    std::vector<Nd4jLong> shapeAt, shapeBt;
    std::vector<Nd4jLong> zShape = ShapeUtils<T>::evalShapeForTensorDot(xShapeInfo, yShapeInfo, {xLastDim}, {yLastButOneDim}, permutAt, permutBt, shapeAt, shapeBt);

    printf("!!! %i  %i\n", xLastDim, yLastButOneDim);
    for(auto item : zShape)
        printf("%i  ", (int)item);
    printf("\n");

    Nd4jLong* zShapeInfo = nullptr;
    ALLOCATE(zShapeInfo, block.getWorkspace(), shape::shapeInfoLength(xRank), Nd4jLong);

    zShapeInfo[0] = xRank;
    for(int i = 0; i < xRank; ++i)
        zShapeInfo[i + 1] = zShape[i];
    shape::updateStrides(zShapeInfo, 'f' /*shape::order(xShapeInfo)*/);    

    return SHAPELIST(zShapeInfo);
}

}
}

#endif


   // CUSTOM_OP_IMPL(matmul, 2, 1, false, -2, -2) {
   //          NDArray<T> *x = INPUT_VARIABLE(0);
   //          NDArray<T> *y = INPUT_VARIABLE(1);
   //          NDArray<T> *z = OUTPUT_VARIABLE(0);

   //          REQUIRE_TRUE(x->rankOf() <= 2 && y->rankOf() <= 2 && z->rankOf() <= 2, 0, "MatMul: Input and Output NDArrays should have rank less or equal to 2");

   //          int iSize = (int) block.getIArguments()->size();
   //          int transA = 0;
   //          int transB = 0;

   //          if (iSize > 0)
   //              transA = INT_ARG(0);

   //          if (iSize > 1)
   //              transB = INT_ARG(1);

   //          T alpha = (T) 1.0f;
   //          T beta = (T) 0.0f;
   //          if (block.getTArguments()->size() > 0)
   //              alpha = block.getTArguments()->at(0);

   //          if (block.getTArguments()->size() > 1)
   //              beta = block.getTArguments()->at(1);


   //          if (transA == 0)
   //              transA = 111;

   //          if (transB == 0)
   //              transB = 111;

   //          if (transA == 1)
   //              transA = 112;

   //          if (transB == 1)
   //              transB = 112;

   //          REQUIRE_TRUE((transA == 111 || transA == 112) && (transB == 111 || transB == 112), 0, "BatchedGemm: valid values for transA and transB are: 0/1 or 111/112, for NoTrans/Trans respectively")
   //          if (x->rankOf() == 1 && y->isMatrix()) {
   //              NDArray<T> *_x = x->reshape(x->ordering(), {1, (int) x->lengthOf()});
   //              NDArray<T> *_y = transB == 111 ? y : y->transpose();
   //              //NDArray<T> *_z = z->reshape(z->ordering(), {1, (int) z->lengthOf()});
        
   //              // gemm
   //              nd4j::NDArrayFactory<T>::mmulHelper(_x, _y, z, alpha, beta);

   //              delete _x;
   //              //delete _z;

   //              if (transB == 112)
   //                  delete _y;
   //          } else if (x->isMatrix() && y->isVector()) {
   //              NDArray<T> *_x = transA == 111 ? x : x->transpose();
   //              NDArray<T> *_y = transB == 111 ? y : y->transpose();
   //              // gemv
   //              nd4j::NDArrayFactory<T>::mmulHelper(_x, _y, z, alpha, beta);

   //              if (transA == 112)
   //                  delete _x;

   //              if (transB == 112)
   //                  delete _y;
   //          } else if (x->isVector() && y->isMatrix() && iSize > 0) {
   //              // gemm
   //              NDArray<T> *_x = transA == 111 ? x : x->transpose();
   //              NDArray<T> *_y = transB == 111 ? y : y->transpose();

   //              nd4j::NDArrayFactory<T>::mmulHelper(_x, _y, z, alpha, beta);

   //              if (transA == 112)
   //                  delete _x;

   //              if (transB == 112)
   //                  delete _y;
   //          } else if (x->isVector() && y->isMatrix()) {
   //              // gemm
   //              nd4j::NDArrayFactory<T>::mmulHelper(x, y, z, alpha, beta);
   //          } else if ((x->isMatrix() && y->isMatrix() || (x->isColumnVector() || (x->isRowVector() && transA == 112)) && (y->isRowVector() || (y->isColumnVector() && transB == 112))) && iSize > 0) {
   //              // gemm
   //              NDArray<T> *_x = transA == 111 ? x : x->transpose();
   //              NDArray<T> *_y = transB == 111 ? y : y->transpose();

   //              REQUIRE_TRUE(_x->rankOf() == 2 && _y->rankOf() == 2, 0, "MatMul: both operands should have rank 2");
   //              REQUIRE_TRUE(_x->columns() == _y->rows(), 0, "MatMul: number of A.colums() should be equal to number of B.rows()");

   //              nd4j::NDArrayFactory<T>::mmulHelper(_x, _y, z, alpha, beta);

   //              if (transA == 112)
   //                  delete _x;

   //              if (transB == 112)
   //                  delete _y;
   //          } else if ((x->isMatrix() && y->isMatrix()) || (x->isColumnVector() && y->isRowVector())) {
   //              // gemm

   //              REQUIRE_TRUE(x->rankOf() == 2 && y->rankOf() == 2, 0, "MatMul: both operands should have rank 2");
   //              REQUIRE_TRUE(x->columns() == y->rows(), 0, "MatMul: number of A.colums() should be equal to number of B.rows()");

   //              nd4j::NDArrayFactory<T>::mmulHelper(x, y, z, alpha, beta);
   //          } else if (x->isVector() && y->isVector()) {
   //              // dot
   //              nd4j::NDArrayFactory<T>::mmulHelper(x, y, z, alpha, beta);
   //          } else if (x->isVector() && y->isScalar()) {
   //              // elementwise mul

   //              x->template applyScalar<simdOps::Multiply<T>>(y->getScalar(0), z, nullptr);
   //           } else if (x->isScalar() && y->isVector()) {
   //              // elementwise mul, reverse op

   //              y->template applyScalar<simdOps::Multiply<T>>(x->getScalar(0), z, nullptr);
   //          }

   //          STORE_RESULT(*z);

   //          return ND4J_STATUS_OK;
   //      }
   //      DECLARE_SYN(mMul, matmul);
   //      DECLARE_SYN(mmul, matmul);
   //      DECLARE_SYN(gemm, matmul);
   //      DECLARE_SYN(gemv, matmul);
   //      DECLARE_SYN(dot, matmul);

   //      DECLARE_SHAPE_FN(matmul) {

   //          int iSize = (int) block.getIArguments()->size();
   //          int transA = 0;
   //          int transB = 0;

   //          if (iSize > 0)
   //              transA = INT_ARG(0);

   //          if (iSize > 1)
   //              transB = INT_ARG(1);

   //          if (transA == 0)
   //              transA = 111;

   //          if (transB == 0)
   //              transB = 111;

   //          if (transA == 1)
   //              transA = 112;

   //          if (transB == 1)
   //              transB = 112;

   //          auto outputShape = ShapeUtils<T>::matrixProductShape(inputShape->at(0), inputShape->at(1), transA == 112, transB == 112, block.getWorkspace());

   //          return SHAPELIST(outputShape);
   //      }