#include <ml.h>
#include <cv.h>
#include <iostream>


int main(int argc, char *argv[])
{
  // pretty simple example of taking 10 feature vectors where the first
  // 5 are labeled with the value '1' and the second five are labeled
  // with the value '2'.  Then the SVM is trained using 8 of the 10
  // vectors and the other 2 are then used to test the predict function
  // of the SVM.  The vectors all contain 5 dimensions, however to
  // demonstrate the use of varIdx, the 3rd dimension is ignored effectively
  // making all the vectors 4 dimensional.  Hope this helps
  // 
  // -Josh
  
  // autotrain or train without autotrain
  const bool AUTOTRAIN = true;

  float data[] = {1.2, 1.4, 5.4, 10.6, 1.2,
                  0.4, 1.9, 5.3, 9.3 , 1.4,
                  2.3, 1.7, 5.1, 11.3, 1.8,
                  1.7, 2.1, 6.0, 10.1, 0.6,
                  3.1, 1.3, 8.0, 10.9, 1.6,   // masked off by sampleIdx (sampMask)
                  6.3, 3.7, 5.3, 12.6, 3.6,
                  7.1, 3.3, 5.1, 8.6 , 2.9,
                  6.6, 2.9, 4.9, 9.6 , 4.0,
                  6.3, 3.1, 6.4, 9.3 , 3.0,
                  5.9, 3.8, 5.6, 12.6, 3.3};  // masked off by sampleIdx (sampMask)
                      //     ^
                      //     ^
                      //     ^ This element is masked off by varIdx (varMask)

  int resp[] = {1,    // class 1
                1,
                1,
                1,
                1,
                2,    // class 2
                2,
                2,
                2,
                2};

  // treat as 4 element vector ignoring column 3
  uchar varMask[] = {1, 1, 0, 1, 1};

  uchar sampMask[] = {1,
                      1,
                      1,
                      1,
                      0,    // dont train with this
                      1,
                      1,
                      1,
                      1,
                      0};   // or this

  // trainData is the matrix containing your descriptor vectors
  // each line is one descriptor
  // I'm not sure but I think this must be of type CV_32FC1
  CvMat trainData = cvMat( 10, 5, CV_32FC1, data );

  // the responses act as labels for your vectors, for example
  // if the first five are labeled "People" then you want the first
  // 5 rows in the response to be the same value, then if the seond
  // five are labeled "Other", you want the second 5 values to be
  // some other value.
  CvMat responses = cvMat( 10, 1, CV_32SC1, resp );
  
  // varIdx supposedly masks off values from your descriptor vector,
  // for example if you wanted to use a 4 element vector instead of
  // five by ignoring the 3rd element, varIdx would look somethin
  // like [1 1 0 1 1], zero values should be masked off.
  //
  // WARNING: passing this to the some classifiers in OpenCV causes
  // segmentation faults, so I don't recommend using it often.  If
  // you don't want to mask any variables off pass 0 instead.
  CvMat varIdx = cvMat( 1, 5, CV_8UC1, varMask );
  
  // sampleIdx is similar to varIdx except it masks off entire feature
  // vectors so when you train you don't use these values, this actually
  // does work in OpenCV.  All zero value rows are masked off.
  // Note: This seems to work properly for all OpenCV classifiers.
  CvMat sampleIdx = cvMat( 10, 1, CV_8UC1, sampMask );

  // CvSVMParams svm_type and kernel_type values
  // ###################
  // # SVM params info #
  // ## Classification #
  // #    C_SVC        #
  // #    NU_SVC       #
  // #    ONE_CLASS    #
  // ## Regression #####
  // #    EPS_SVR      #
  // #    NU_SVR       #
  // ###################
  //
  // # kernel types #########
  // #    LINEAR            #
  // #    POLY              # (Does not work with EPS_SVR or NU_SVR)
  // #    RBF               # (Does not work with ESP_SVR)
  // #    SIGMOID           #
  // ########################

  // this contains all the Svm parameters
  CvSVMParams parameters(
      CvSVM::C_SVC,   // svm_type (regression types return floating points)
      CvSVM::LINEAR,  // kernel_type
      3,              // degree
      0.1,            // gamma
      0,              // coeficient 0
      1,              // C
      0.5,            // nu
      0.1,            // p
      0,              // this is a CvMat* which weights the responses
      cvTermCriteria( // termination criteria (don't remember what this is)
        CV_TERMCRIT_ITER + CV_TERMCRIT_EPS,  // type
        50,                                  // max_iter
        0.1                                  // epsilon
      )
    );

  // Declare the actual SVM
  CvSVM svm;

  // Train the SVM
 
  std::cout << "Training..." << std::endl;

  // two different methods of training, train_auto determines the parameters
  // for the SVM automatically, regular train does not.
  if ( !AUTOTRAIN ) // manual train
  {
    svm.train( &trainData, &responses, &varIdx, &sampleIdx, parameters );
  }
  else    // autotrain the classifier
  {
    int kFolds = 2;     // number of k-folds
    svm.train_auto( &trainData, &responses, &varIdx, &sampleIdx, parameters, kFolds );
  }

  // Test the SVM

  std::cout << "Testing..." << std::endl;

  // now lets predict some unknown values, predict takes a single decriptor vector
  // so it needs to be the same number of columns as trainData, but it has only 1
  // row
  CvMat* predictMat = cvCreateMat( 1, 5, CV_32FC1 );

  // copy the 5th (masked off) row of trainData into the prediction matrix
  for ( int i = 0; i < 5; i++ )
    cvSet2D( predictMat, 0, i, cvGet2D( &trainData, 4, i ) );

  double respPrediction;
  
  // predict the classification of the feature vector, this will
  // value from your responses matrix (in this case 1 or 2), for
  // regression types, it will return intermediate floating point
  // values
  respPrediction = svm.predict( predictMat );

  std::cout << "Response for first vector is " << respPrediction << std::endl;

  // copy the 10th (masked off) row of trainData into the prediction matrix
  for ( int i = 0; i < 5; i++ )
    cvSet2D( predictMat, 0, i, cvGet2D( &trainData, 9, i ) );
  
  // predict the next vector
  respPrediction = svm.predict( predictMat );
  
  std::cout << "Response for the second vector is " << respPrediction << std::endl;

  // clean up
  cvReleaseMat( &predictMat );

  return 0;
}
