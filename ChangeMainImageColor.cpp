#include <iostream>
#include <algorithm>
#include <vector>
#include <filesystem>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

vector<string> LoadInputImageFilenames() {
	// InputImagesフォルダが存在しない場合は作成する
	if (!filesystem::exists("InputImages")) {
		filesystem::create_directory("InputImages");
	}

	// InputImagesフォルダ内のすべてのファイルを取得
	vector<string> filenames;
	for (const auto& entry : filesystem::directory_iterator("InputImages")) {
		filenames.push_back(entry.path().string());
	}
	return filenames;
}

// InputImagesフォルダ内の画像ファイルを取得する
vector<Mat> LoadInputImages(vector<string> filenames) {
	// 取得した画像ファイルをInputImages配列に格納
	vector<Mat> InputImages;
    for (const auto& filename : filenames) {
		Mat image = imread(filename, -1);
        if (image.empty()) {
			cerr << "Error: 画像ファイル " << filename << " を読み込めませんでした。" << endl;
			return InputImages;
		}
		InputImages.push_back(image);
	}
	return InputImages;
}

//色を比較する
bool CompareColor(int color1, int color2, int allowableError) {
	return abs(color1 - color2) >= allowableError && color1 < color2;
}
//色相の許容率を計算
int CalculateHueToleranceRate(int b, int g, int r, int toleranceRate) {
	auto minmax = std::minmax({ b, g, r });
	int def = abs(minmax.second - minmax.first);
	return def * toleranceRate / 100;
}
string CategorizeByColor(int blue, int green, int red) {
	// カラーコードの最大値
	const int maxColorCode = 255;
	// カラーコードの最小値
	const int minColorCode = 0;
	// 色相の許容率
	const int hueToleranceRate = 35;
	// 明度の許容率
	const int brightnessToleranceRate = 10;
	// 色相の許容値
	const int hueTolerance = CalculateHueToleranceRate(blue, green, red, hueToleranceRate);
	// 明度の許容値
	const int brightnessTolerance = maxColorCode * brightnessToleranceRate / 100;

	// 色の重みを計算
	int blueWeight = 1 - CompareColor(blue, red, hueTolerance) - CompareColor(blue, green, hueTolerance);
	int greenWeight = 1 - CompareColor(green, red, hueTolerance) - CompareColor(green, blue, hueTolerance);
	int redWeight = 1 - CompareColor(red, blue, hueTolerance) - CompareColor(red, green, hueTolerance);
	vector<int> weights = { blueWeight, greenWeight, redWeight };

	// weightsに0が2つ含まれる場合
	if (std::count_if(weights.begin(), weights.end(), [](int weight) { return weight == 0; }) == 2) {
		// すべての要素に対して処理
		std::for_each(weights.begin(), weights.end(), [](int& weight) {
			// 要素から1を引いた値を要素に加える
			weight += weight - 1;
		});
	}
	

	// 重みがすべて同じ場合
	if (blue == green && green == red) {
		if (blue <= minColorCode + brightnessTolerance) {
			return "-1,-1,-1";
		}
		else if (blue >= maxColorCode - brightnessTolerance) {
			return "1,1,1";
		}
		else {
			return "0,0,0";
		}
	}

	// 重みがすべて同じでない場合
	return to_string(weights[0]) + "," + to_string(weights[1]) + "," + to_string(weights[2]);
}
vector<int> ComvertCategoryToWeights(string category) {
	// カテゴリを分割
	vector<string> categorySplit;
	stringstream ss(category);
	string buffer;
	while (getline(ss, buffer, ',')) {
		categorySplit.push_back(buffer);
	}
	// カテゴリの重みを取得
	int blueWeight = stoi(categorySplit[0]);
	int greenWeight = stoi(categorySplit[1]);
	int redWeight = stoi(categorySplit[2]);
	// 重みを返す
	return { blueWeight, greenWeight, redWeight };
}
Mat EditImage(Mat image, uint32_t referencedColor) {
	// 画像のサイズを取得
	int width = image.cols;
	int height = image.rows;

	// カテゴリごとにピクセルの座標を格納する
	map<string, vector<vector<int>>> categorizedPixelsCoordinates;
	// 最も要素数の多いカテゴリ
	string maxCategory;
	// 最も要素数の多いカテゴリの要素数
	int maxCategoryCount = 0;
	// 最も要素数の多いカテゴリの重み
	vector<int> maxCategoryWeights;

	// 参照する色のカテゴリを取得
	int referencedBlue = referencedColor & 0xFF;
	int referencedGreen = (referencedColor >> 8) & 0xFF;
	int referencedRed = (referencedColor >> 16) & 0xFF;
	string referencedCategory = CategorizeByColor(referencedBlue, referencedGreen, referencedRed);
	// 参照する色のカテゴリの重みを取得
	vector<int> referencedWeights = ComvertCategoryToWeights(referencedCategory);

	// 画像のすべてのピクセルを走査
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// アルファ値が0の場合は処理をスキップ
			if (image.at<Vec4b>(y, x)[3] == 0) continue;

			// ピクセルの色を取得
			Vec4b pixel = image.at<Vec4b>(y, x);
		
			// 色の値を取得
			int blue = pixel[0];
			int green = pixel[1];
			int red = pixel[2];

			// 色がグレースケールの場合は処理をスキップ
			if (blue == green && green == red) continue;

			// カテゴリを取得
			string category = CategorizeByColor(blue, green, red);
			
			// カテゴリごとにピクセルの座標を格納
			categorizedPixelsCoordinates[category].push_back({ x, y });
		}
	}

	// 各カテゴリの中で最も要素数が多いカテゴリを取得
	for (const auto& [category, pixels] : categorizedPixelsCoordinates) {
		if (pixels.size() > maxCategoryCount) {
			maxCategory = category;
			maxCategoryCount = pixels.size();
		}
	}

	// 最も要素数が多いカテゴリの重みを取得
	maxCategoryWeights = ComvertCategoryToWeights(maxCategory);

	// 最も要素数が多いカテゴリを走査
	for (const auto& pixelCoordinates : categorizedPixelsCoordinates[maxCategory]) {
		// ピクセルの座標を取得
		int x = pixelCoordinates[0];
		int y = pixelCoordinates[1];
		// ピクセルの色を取得
		Vec4b pixel = image.at<Vec4b>(y, x);
		// 重みとRGB値の対応を格納する
		map<int, int> weightToRGB;
		// RGB値のそれぞれの重み
		int blueWeight = maxCategoryWeights[0];
		int greenWeight = maxCategoryWeights[1];
		int redWeight = maxCategoryWeights[2];

		// 重みとRGB値の対応を設定
		weightToRGB[blueWeight] = pixel[0];
		weightToRGB[greenWeight] = pixel[1];
		weightToRGB[redWeight] = pixel[2];

		// 重みに0が含まれないとき
		if (blueWeight * greenWeight * redWeight != 0)
			weightToRGB[0] = (weightToRGB[1] + weightToRGB[-1]) / 2;

		// 参照する色の重みと最も要素数が多いカテゴリの重みが等しくなるようにRGB値を入れ替える
		pixel[0] = weightToRGB[referencedWeights[0]];
		pixel[1] = weightToRGB[referencedWeights[1]];
		pixel[2] = weightToRGB[referencedWeights[2]];
		// 画像にピクセルの色を設定
		image.at<Vec4b>(y, x) = pixel;
	}

	// 画像を返す
	return image;
}

string GetOutputImageFilename(string inputImageFilename) {
	// ファイル名を取得
	string filename = inputImageFilename.substr(inputImageFilename.find_last_of("/\\") + 1);
	// 拡張子を取得
	string extension = filename.substr(filename.find_last_of(".") + 1);
	// 拡張子を除いたファイル名を取得
	string filenameWithoutExtension = filename.substr(0, filename.find_last_of("."));
	// 出力ファイル名を返す
	return "OutputImages/" + filenameWithoutExtension + "_processed." + extension;
}
int main() {
	// 参照する色
	uint32_t referencedColor = 0x85A66F;
	// InputImagesフォルダ内の画像ファイルを取得
	vector<string> filenames = LoadInputImageFilenames();
	// InputImagesフォルダ内の画像ファイルを取得
	vector<Mat> images = LoadInputImages(filenames);

	// 画像ファイルを取得できなかった場合は終了
	if (images.empty()) {
		cerr << "Error: 画像ファイルを取得できませんでした。" << endl;
		return -1;
	}

	// 画像を処理
	for (int i = 0; i < images.size(); i++) {
		// 画像を処理
		Mat editedImage = EditImage(images[i], referencedColor);

		// OutputImagesフォルダが存在しない場合は作成する
		if (!filesystem::exists("OutputImages")) {
			filesystem::create_directory("OutputImages");
		}

		// 画像を保存
		string outputFilename = GetOutputImageFilename(filenames[i]);
		imwrite(outputFilename, editedImage);
		cout << "画像を保存しました: " << outputFilename << endl;
	}
	return 0;
}
