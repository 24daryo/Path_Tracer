#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <string>
#include <math.h>

using namespace std;

class Math
{
public:
    static void println(string word)
    {
    }

    static float Pow2(float t)
    {
        return t * t;
    }
};

//ベクトルを定義
class Vector3
{
public:
    //三次元
    double x, y, z;
    //コンストラクタ(初期化)
    Vector3(double x_ = 0, double y_ = 0, double z_ = 0)
    {
        x = x_;
        y = y_;
        z = z_;
    }

    //オペレータ(ベクトル演算を可能にする)
    Vector3 operator+(const Vector3 &b) const { return Vector3(x + b.x, y + b.y, z + b.z); }
    Vector3 operator-(const Vector3 &b) const { return Vector3(x - b.x, y - b.y, z - b.z); }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }
    Vector3 operator*(double b) const { return Vector3(x * b, y * b, z * b); }
    //アダマール積
    Vector3 Mult(const Vector3 &b) const { return Vector3(x * b.x, y * b.y, z * b.z); }
    //正規化
    Vector3 &Norm() { return *this = *this * (1 / sqrt(x * x + y * y + z * z)); }
    //内積
    double Dot(const Vector3 &b) const { return x * b.x + y * b.y + z * b.z; }
    //外積
    Vector3 Cross(Vector3 &b) { return Vector3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }

    //表示
    void Print()
    {
        printf("( %.3f, %.3f, %.3f)", x, y, z);
    }
    //セット
    Vector3 Set(double x_, double y_, double z_)
    {
        x = x_;
        y = y_;
        z = z_;
        return *this;
    }
};

//光線
class Ray
{
public:
    //変数
    Vector3 origin;    //中心座標
    Vector3 direction; //方向
};

//球体
class Sphere
{
public:
    //変数
    Vector3 center; //中心座標
    float radius;   //半径

    //初期化
    Sphere(Vector3 _center, float _radius)
    {
        center = _center;
        radius = _radius;
    }
    bool Hit(Sphere s, Ray r)
    {
        Vector3 oc = r.origin - s.center;
        float a = (r.direction).Dot(r.direction);
        float b = oc.Dot(r.direction);
        float c = oc.Dot(oc) - s.radius * s.radius;
        float discriminant = b * b - a * c;
        float t;
        if (discriminant > 0)
        {
            t = (-b - sqrt(discriminant)) / a; //解１：手前側
            return true;
        }
        return false;
    }
};

// 視界の管理クラス
class Camera
{
private:
    Vector3 X, Y, Z;
    Vector3 screenOrigin;

public:
    Vector3 eye;                   //視点
    Vector3 lookAt;                //視線方向
    double angle = 45;             //アングル
    int width = 400, height = 300; //ピクセル数
    Vector3 viewUp;                //上向きのベクトル

    //初期化
    Camera()
    {
        viewUp.Set(0, 0, 1);
        eye.Set(0, 1, 0);
        lookAt.Set(1, 1, -0.5).Norm();
    }

    void Reset()
    {
        //スクリーンまでの奥行きを求める
        double depth = (height / 2) / tan(angle);
        //スクリーン中心の座標を求める
        Vector3 screenCenter = eye + lookAt * depth;
        //図のX,Y,Zベクトルを求める
        Z = -lookAt.Norm();
        X = viewUp.Cross(Z).Norm(); //viewUpとZの外積を求めて正規化
        Y = Z.Cross(X).Norm();      //ZとXの外積を求めて正規化
        //スクリーンの左上の座標を求める
        screenOrigin = screenCenter + Y * (height / 2) - Z * (width / 2);
    }

    //uv座標の(i,j)の場所の光線を取得する
    Ray GetRay(int i, int j)
    {
        //光線の場所を求める
        Vector3 rayPos = screenOrigin + X * i + Y * j;
        //光線をリターンする
        Ray ray;
        ray.direction = rayPos - eye;
        ray.origin = eye;
        return ray;
    }
};

class Color
{
public:
    int a, g, b;
    Color(Vector3 v)
    {
        v.Norm();
        a = (int)(255 * v.x);
        a = (int)(255 * v.x);
        a = (int)(255 * v.x);
    }
};

class World
{
public:
    Color BackImage(Ray ray)
    {
        //光線の方向で色を決定する
        Color color(ray.direction);
        return color;
    }

    void SetCamera()
    {
        Camera camera;
    }

    void GetImage()
    {
    }
};

int main()
{
    printf("tesuto");
    /*============================*/
    /*   カラー画像の書き込み     */
    /*============================*/
    FILE *fp;
    fp = fopen("./test.ppm", "wb"); //■ write binaryモードで画像ファイルを開く
    fprintf(fp, "P6\n");            //■ ファイルの識別符号を書き込む
    fprintf(fp, "128 128\n");       //■ 画像サイズを書き込む
    fprintf(fp, "255\n");           //■ 最大輝度値を書き込む

    //■ 画像データの書き込み
    for (int y = 0; y < 100; y++)
    {
        for (int x = 0; x < 100; x++)
        {
            int r = 255; /* 赤成分 ( 0 〜 255 ) */
            int g = 255; /* 緑成分 ( 0 〜 255 ) */
            int b = 255; /* 青成分 ( 0 〜 255 ) */
            fprintf(fp, "%d %d %d ", r, g, b);
        }
    }
    fclose(fp);

    return 0;
}