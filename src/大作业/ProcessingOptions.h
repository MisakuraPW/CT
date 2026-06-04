#pragma once

struct PreprocessingOptions
{
	double lungWindowLevel = -600.0;
	double lungWindowWidth = 1500.0;
	int gaussianKernelSize = 5;
	int medianKernelSize = 5;
	double claheClipLimit = 2.0;
	int claheTileGridSize = 8;
};

struct LungSegmentationOptions
{
	int thresholdGaussianKernelSize = 5;
	int minComponentArea = 64;
	int minComponentAreaDivisor = 1000;
	int keepComponentCount = 2;
	int openKernelSize = 3;
	int closeKernelSize = 7;
	int morphologyIterations = 1;
};
