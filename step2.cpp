#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define WIDTH 400  //横のピクセル数
#define HEIGHT 300 //縦のピクセル数
#define PI 3.14159 //円周率

//ベクトルの定義
class Vector3
{
public:
    //三次元
    double x, y, z;

    //コンストラクタ(初期化)
    Vector3(double _x = 0, double _y = 0, double _z = 0)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    //セット
    Vector3 Set(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
        return *this;
    }
    //オペレータ(ベクトル演算を可能にする)
    Vector3 operator+(const Vector3 &b) const { return Vector3(x + b.x, y + b.y, z + b.z); }
    Vector3 operator-(const Vector3 &b) const { return Vector3(x - b.x, y - b.y, z - b.z); }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }
    Vector3 operator*(double b) const { return Vector3(x * b, y * b, z * b); }
    //正規化
    Vector3 &Norm() { return *this = *this * (1 / sqrt(x * x + y * y + z * z)); }
    //内積
    double Dot(const Vector3 &b) const { return x * b.x + y * b.y + z * b.z; }
    //外積
    Vector3 Cross(const Vector3 &b) { return Vector3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    //長さ
    float Length() { return sqrt(x * x + y * y + z * z); }
    //表示
    void Print() { printf("( %.3f, %.3f, %.3f)\n", x, y, z); }
};

//光線
class Ray
{
public:
    //変数
    Vector3 origin;    //中心座標
    Vector3 direction; //方向
    //初期化
    Ray(Vector3 o, Vector3 d)
    {
        origin = o;
        direction = d;
    }
    //セット
    void Set(Vector3 o, Vector3 d)
    {
        origin = o;
        direction = d;
    }
};
//カラークラス
class Color
{
public:
    int r, g, b;
    Color() {} //クラスでの宣言で必要
    //基本はこちらを使用
    Color(int _r, int _g, int _b)
    {
        r = _r;
        g = _g;
        b = _b;
    }
    //セット
    void Set(int _r, int _g, int _b)
    {
        r = _r;
        g = _g;
        b = _b;
    }
    Color VectorToColor(Vector3 v)
    {
        v.Norm();
        //虹色に色付けする
        Vector3 colorVector = (v + Vector3(1, 1, 1)) * 0.5;
        r = (int)(255 * colorVector.x);
        g = (int)(255 * colorVector.y);
        b = (int)(255 * colorVector.z);
        return Color(r, g, b);
    }
    //加算
    Color operator+(const Color &left) const
    {
        int R = r + left.r;
        int G = g + left.g;
        int B = b + left.b;
        if (R > 255)
            R = 255;
        if (G > 255)
            G = 255;
        if (B > 255)
            B = 255;
        return Color(R, G, B);
    }
    //減算
    Color operator-(const Color &left) const
    {
        int R = r - left.r;
        int G = g - left.g;
        int B = b - left.b;
        if (R < 0)
            R = 0;
        if (G < 0)
            G = 0;
        if (B < 0)
            B = 0;
        return Color(R, G, B);
    }
    Color operator*(const float B) const
    {
        return Color((int)(B * r), (int)(B * g), (int)(B * b));
    }
    Color operator*(const Color &left) const
    {
        return Color(
            (int)(r * left.r / 255),
            (int)(g * left.g / 255),
            (int)(b * left.b / 255));
    }
    Color operator/(const double t) const
    {
        return Color((int)(r / t), (int)(g / t), (int)(b / t));
    }
};

class Sphere
{
public:
};

class Camera
{
private:
    Vector3 X, Y, Z;      //カメラ基準の方向ベクトル
    Vector3 screenOrigin; //スクリーンの原点

public:
    Vector3 eye;    //視点
    Vector3 lookAt; //方向
    double angle;   //アングル

    //初期化
    void Set(Vector3 _eye, Vector3 _lookAt, double _angle)
    {
        eye = _eye;
        lookAt = _lookAt;
        angle = _angle;
        SetScreenOrigin();
    }

    //uv座標の(i,j)の場所の光線を取得する
    Ray GetScreenRay(int i, int j)
    {
        //光線の場所を求める
        Vector3 rayPos = screenOrigin + X * i + Y * j;
        //光線をリターンする
        return Ray(eye, (rayPos - eye).Norm());
    }

private:
    void SetScreenOrigin()
    {
        //スクリーンまでの奥行きを求める
        double depth = (HEIGHT / 2) / tan(angle * (PI / 180));
        //スクリーン中心の座標を求める
        Vector3 screenCenter = eye + lookAt.Norm() * depth;
        //図のX,Y,Zベクトルを求める
        Z = -lookAt.Norm();
        X = -Vector3(0, 1, 0).Cross(Z).Norm(); //上向きベクトルとZの外積を求めて正規化
        Y = Z.Cross(X).Norm();                 //ZとXの外積を求めて正規化
        //スクリーン上での原点の座標を求める
        screenOrigin = screenCenter - Y * (HEIGHT / 2) - X * (WIDTH / 2);
    }
};

class World
{
public:
    //カメラ
    Camera camera; //初期化
    World()
    {
        //カメラを設定
        camera.Set(Vector3(0, 0, 0), Vector3(0, 0, 1), 45); //座標、視線方向、仰角
    }

    //画像生成
    void GetImage()
    {
        // カラー画像の書き込み 　　
        FILE *fp;
        fp = fopen("./test1.ppm", "wb");       //■ write binaryモードで画像ファイルを開く
        fprintf(fp, "P3\n");                   //■ ファイルの識別符号を書き込む
        fprintf(fp, "%d %d\n", WIDTH, HEIGHT); //■ 画像サイズを書き込む
        fprintf(fp, "%d\n", 255);              //■ 最大輝度値を書き込む
                                               //■ 画像データの書き込み
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                //カメラからの光線を取得する
                Ray cameraRay = camera.GetScreenRay(x, y);
                //光線を飛ばして色を取得し加算
                Color c = GetColor(cameraRay); //取得した色を書き込む
                fprintf(fp, "%d %d %d ", c.r, c.r, c.b);
            }
        }
        fclose(fp);
    }

    //光線を飛ばして色を取得する
    Color GetColor(Ray inputRay)
    {
        return Color();
    }
};

int main()
{
    World world;
    world.GetImage();
    return 0;
}
