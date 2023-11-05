using System.Drawing;
using System.Drawing.Imaging;
using System.Diagnostics;
using static System.Net.Mime.MediaTypeNames;
using System.Globalization;

namespace testProject {
    internal class GetWeaponImg {
    //GetImg
        static void GetImg() {
            String imgName = "miku2.png";
            String imgPath = "C:\\Users\\qaz2w\\IT\\SorceCode\\cs\\testProject\\testProject\\Imgs\\";
            String preImgPath = imgPath + "Pre\\" + imgName;
            String postImgPath = imgPath + "Post\\" + imgName;
            Bitmap preImg;

            try {
                preImg = new Bitmap(preImgPath);
            } catch (Exception e) {
                Console.WriteLine(e.Message);
                return;
            }

            if (preImg == null) return;
            Color postColor = Color.Green;

            byte postR = postColor.R;
            byte postG = postColor.G;
            byte postB = postColor.B;
            byte[] postRgb = { postR, postG, postB };

            int[] postRgbRank = RgbRank(postColor);
            int[] mainRgbRank = GetMainRgbRank(preImg);
            int mainRgbRankValue = mainRgbRank[0] * 100 + mainRgbRank[1] * 10 + mainRgbRank[2];

            int j = 0;
            for (int x = 0; x < preImg.Width; x++) {
                for (int y = 0; y < preImg.Height; y++) {
                    Color preColor = preImg.GetPixel(x, y);
                    byte preR = preColor.R;
                    byte preG = preColor.G;
                    byte preB = preColor.B;
                    int preRgbRankValue;
                    int preRRank;
                    int preGRank;
                    int preBRank;
                    int postRRank;
                    int postGRank;
                    int postBRank;
                    int[] preRgbRank = RgbRank(preColor);

                    bool isSimilarColor = IsSimilarColor(preColor, preRgbRank, mainRgbRank);



                    preRRank = preRgbRank[0];
                    preGRank = preRgbRank[1];
                    preBRank = preRgbRank[2];
                    postRRank = postRgbRank[0];
                    postGRank = postRgbRank[1];
                    postBRank = postRgbRank[2];

                    preRgbRankValue = preRRank * 100 + preGRank * 10 + preBRank;

                    if (preRgbRankValue != mainRgbRankValue && !isSimilarColor) continue;

                    postRgb[preRRank] = preR;
                    postRgb[preGRank] = preG;
                    postRgb[preBRank] = preB;


                    preImg.SetPixel(x, y, Color.FromArgb(preColor.A, postRgb[postRRank], postRgb[postGRank], postRgb[postBRank]));
                }
            }
            preImg.Save(postImgPath, System.Drawing.Imaging.ImageFormat.Png);
            preImg.Dispose();
        }

        private static int[] GetMainRgbRank(Bitmap img) {
            Dictionary<Color, int> colorCount = new Dictionary<Color, int>();
            int imgWidth = img.Width;
            int imgHeight = img.Height;
            int resizeWidth = 50;
            int resizeHeight = (int)(imgHeight * ((double)resizeWidth / (double)imgWidth));
            Bitmap resizedImage = new Bitmap(imgWidth, imgHeight);
            Graphics graphics = Graphics.FromImage(resizedImage);
            graphics.DrawImage(img, 0, 0, resizeWidth, resizeHeight);
            graphics.Dispose();
            for (int x = 0; x < resizeWidth; x++) {
                for (int y = 0; y < resizeHeight; y++) {

                    Color pixelColor = resizedImage.GetPixel(x, y);
                    int a = pixelColor.A;
                    float r = pixelColor.R;
                    float g = pixelColor.G;
                    float b = pixelColor.B;
                    if (a < 240) continue;
                    if (1.1 > r / g && r / g > 0.9 && 1.1 > r / b && r / b > 0.9) continue;

                    if (!colorCount.ContainsKey(pixelColor)) colorCount.Add(pixelColor, 0);


                    colorCount[pixelColor]++;
                }
            }

            Color mainColor = colorCount.OrderByDescending(x => x.Value).FirstOrDefault().Key;

            return RgbRank(mainColor);
        }

        private static int[] RgbRank(Color color) {
            int r = color.R;
            int g = color.G;
            int b = color.B;
            int[] rgb = { r, g, b };

            int rRank;
            int gRank;
            int bRank;
            int[] rank = new int[3];

            Array.Sort(rgb);
            rRank = Array.IndexOf(rgb, r); rgb[rRank] = g + b + 15;
            gRank = Array.IndexOf(rgb, g); rgb[gRank] = b - 1;
            bRank = Array.IndexOf(rgb, b);

            rank[0] = rRank; rank[1] = gRank; rank[2] = bRank;

            return rank;
        }
        private static bool IsSimilarColor(Color color, int[] preRgbRank, int[] mainRgbRank) {
            int[] rgb = { color.R, color.G, color.B };
            int mainTop = Array.IndexOf(mainRgbRank, 2);
            int mainMid = Array.IndexOf(mainRgbRank, 1);
            int preTop = Array.IndexOf(preRgbRank, 2);
            int preMid = Array.IndexOf(preRgbRank, 1);
            if (mainTop + mainMid != preTop + preMid || -(mainTop - mainMid) != preTop - preMid) return false;
            int topColor = rgb[preTop];
            int midColor = rgb[preMid];
            float division = (float)midColor / (float)topColor;


            return 1.1 > division;
        }
    }
}
