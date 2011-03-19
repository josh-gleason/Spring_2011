runall: smooth gauss laplacian edges sobel

smooth:
	./smooth/smooth 5 images/lenna.pgm smooth/lenna5.jpg smooth/lenna5ocv.jpg
	./smooth/smooth 11 images/lenna.pgm smooth/lenna11.jpg smooth/lenna11ocv.jpg
	./smooth/smooth 5 images/sf.pgm smooth/sf5.jpg smooth/sf5ocv.jpg
	./smooth/smooth 11 images/sf.pgm smooth/sf11.jpg smooth/sf11ocv.jpg

gauss:
	./gauss/gauss images/lenna.pgm gauss/lennaA_ gauss/lennaB_
	./gauss/gauss images/sf.pgm gauss/sfA_ gauss/sfB_

laplacian:
	./laplacian/laplacian images/lenna.pgm laplacian/lennaA_ laplacian/lennaB_
	./laplacian/laplacian images/sf.pgm laplacian/sfA_ laplacian/sfB_

edges:
	./edges/edges images/lenna.pgm 1 edges/lenna_1_
	./edges/edges images/sf.pgm 1 edges/sf_1_
	./edges/edges images/lenna.pgm 4 edges/lenna_4_
	./edges/edges images/sf.pgm 4 edges/sf_4_

sobel:
	./sobel/sobel images/aerial.pgm 50 sobel/aerial_50_
	./sobel/sobel images/aerial.pgm 100 sobel/aerial_100_
	./sobel/sobel images/aerial.pgm 200 sobel/aerial_200_
	./sobel/sobel images/aerial.pgm 300 sobel/aerial_300_
	./sobel/sobel images/wheel.pgm 50 sobel/wheel_50_
	./sobel/sobel images/wheel.pgm 100 sobel/wheel_100_
	./sobel/sobel images/wheel.pgm 200 sobel/wheel_200_
	./sobel/sobel images/wheel.pgm 400 sobel/wheel_400_

.PHONY: smooth gauss laplacian edges sobel
