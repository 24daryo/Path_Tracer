#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <string>
#include <math.h>

using namespace std;

#define WIDTH 400       //横のピクセル数
#define HEIGHT 300      //縦のピクセル数
#define SAMPLING_SIZE 1 //サンプリング数
#define GROSS 255       //輝度の階級
#define PI 3.14159      //円周率
//#define INFINITY 1000          //無限大
#define MAX_REFLECTSION_SIZE 1 //最大反射回数

//前方宣言
class Color;
//ベクトルの定義
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
    //長さ
    float Length() { return sqrt(x * x + y * y + z * z); }
    //表示
    void Print()
    {
        printf("( %.3f, %.3f, %.3f)\n", x, y, z);
    }
    //セット
    Vector3 Set(double x_, double y_, double z_)
    {
        x = x_;
        y = y_;
        z = z_;
        return *this;
    }
    Vector3 Lerp(double t, Vector3 start, Vector3 end)
    {
        return (start - end) * t;
    }
    Vector3 RandInUnitSphere()
    {
        Vector3 P;
        Vector3 rand(drand48(), drand48(), drand48());
        do
        {
            P = rand * 2 - Vector3(1, 1, 1);
        } while (P.Length() >= 1);
        return P;
    }
    //カラー型に変換
    Color ToColor();
};

class Color
{
public:
    int r, g, b;

    Color() {}
    Color(int _r, int _g, int _b)
    {
        r = _r;
        g = _g;
        b = _b;
    }

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
        r = (int)(GROSS * colorVector.x);
        g = (int)(GROSS * colorVector.y);
        b = (int)(GROSS * colorVector.z);
        return Color(r, g, b);
    }

    Color operator+(const Color &B) const { return Color(r + B.r, g + B.g, b + B.b); }
    Color operator/(const double t) const { return Color((int)(r / t), (int)(g / t), (int)(b / t)); }
};

Color Vector3::ToColor()
{
    Vector3 v(x, y, z);
    v.Norm();
    Color color((int)((v.x + 1) * 255 / 2), (int)((v.y + 1) * 255 / 2), (int)((v.z + 1) * 255 / 2));
    return color;
}

//光線
class Ray
{
public:
    //変数
    Vector3 origin;    //中心座標
    Vector3 direction; //方向
    int reflectCount;  //反射回数

    void Set(Vector3 o, Vector3 d)
    {
        origin = o;
        direction = d;
        reflectCount = 0;
    }
};

enum reflectionType
{
    DIFFUSE,
    REFLECTION,
    REFRACTION
};

class Material
{
public:
    //色
    Color color;
    //反射率
    float albedo;
    //反射タイプ
    reflectionType type;
    //発光
    //float emission;
    //拡散反射
    //float diffuse;
    //鏡面反射
    //float reflection;
    //屈折
    //float refraction;

    void Set(Color _color, reflectionType _type)
    {
        color = _color;
        type = _type;
        albedo = 0.5;
    }
};

//球体
class Sphere
{
public:
    //変数
    Vector3 center;    //中心座標
    float radius;      //半径
    float solveT;      //交点の解
    Material material; //マテリアル

    //初期化
    Sphere()
    {
    }
    //セット
    void Set(Vector3 _center, float _radius, Material _material)
    {
        center = _center;
        radius = _radius;
        material = _material;
    }

    //交差判定
    bool IsHit(Ray ray)
    {
        Vector3 oc = ray.origin - center;
        //二次方程式の定数a,b,cを求める
        float a = (ray.direction).Dot(ray.direction);
        float b = oc.Dot(ray.direction);
        float c = oc.Dot(oc) - radius * radius;
        //判別式Dを求める
        float discriminant = b * b - a * c;
        //交点が存在する場合
        if (discriminant > 0)
        {
            solveT = (-b - sqrt(discriminant)) / a; //解1(手前側)
            //交点が前方に存在する場合
            if (solveT > 0.01)
            {
                return true;
            }
            //交点が前方で奥側に存在する場合(つまり内→外の光線)
            solveT = (-b + sqrt(discriminant)) / a; //解2(奥側)
            if (solveT > 0.01)
            {
                return true;
            }
        }
        return false;
    }
    Color GetNormalRay(Ray ray)
    {
        Ray outputRay;
        //反射位置Pを求める
        Vector3 P = ray.origin + ray.direction * solveT; //P = A + tB
        //法線ベクトルを求める
        Vector3 N = (P - center).Norm();
        return N.ToColor();
    }
    Ray GetRay(Ray ray)
    {
        Ray outputRay;
        //反射位置Pを求める
        Vector3 P = ray.origin + ray.direction * solveT; //P = A + tB
        //法線ベクトルを求める
        Vector3 N = (P - center).Norm();
        if (material.type == DIFFUSE)
        {
            Vector3 target = P + N + Vector3().RandInUnitSphere();
            outputRay.direction = (target - P);
            outputRay.origin = P;
        }
        /*
        else if (material.type == REFLECTION)
        {
        }
        else if (material.type == REFLECTION)
        {
        }
        */
        return outputRay;
    }
};

// 視界の管理クラス
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
    Ray GetRay(int i, int j)
    {
        //光線の場所を求める
        Vector3 rayPos = screenOrigin + X * i + Y * j;
        //光線をリターンする
        Ray ray;
        ray.direction = (rayPos - eye).Norm();
        ray.origin = eye;
        return ray;
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
    Camera camera;
    //マテリアル
    Material mat1;
    //球体オブジェクト
    Sphere ball_1, ball_2, terrain;
    Sphere objects[3];

    //初期化
    World()
    {
        //カメラを設定
        camera.Set(Vector3(0, 0, 0), Vector3(0, 0, -1), 30); //座標、視線方向、仰角
        //マテリアルを設定
        mat1.Set(Color(255, 255, 255), DIFFUSE);
        //オブジェクトを設定
        ball_1.Set(Vector3(0, 0, 2), 1, mat1);
        ball_2.Set(Vector3(0, 0, -3), 1, mat1);
        terrain.Set(Vector3(2, -501, 2), 500, mat1);
        //オブジェクトを登録
        objects[0] = ball_1;
        objects[1] = ball_2;
        objects[2] = terrain;
    }
    //画像生成
    void GetImage()
    {
        printf("tesuto");
        /*============================*/
        /*   カラー画像の書き込み     */
        /*============================*/
        FILE *fp;
        fp = fopen("./output.ppm", "wb");      //■ write binaryモードで画像ファイルを開く
        fprintf(fp, "P3\n");                   //■ ファイルの識別符号を書き込む
        fprintf(fp, "%d %d\n", WIDTH, HEIGHT); //■ 画像サイズを書き込む
        fprintf(fp, "%d\n", GROSS);            //■ 最大輝度値を書き込む

        //■ 画像データの書き込み
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                Color sum;
                for (int n = 0; n < SAMPLING_SIZE; n++)
                {
                    int max = HEIGHT * WIDTH * SAMPLING_SIZE;
                    //printf("完了度：(%d, %d, %d)\n", y, x, n);
                    //カメラからの光線を取得する
                    Ray cameraRay = camera.GetRay(x, y);
                    //光線を飛ばして色を取得し加算
                    sum = CastRay(cameraRay);
                }
                //サンプリングの平均を取得
                Color ave = sum / SAMPLING_SIZE;
                //取得した色を書き込む
                fprintf(fp, "%d %d %d ", ave.r, ave.g, ave.b);
            }
        }
        fclose(fp);
    }

private:
    //世界の背景を取得
    Color BackImage(Ray ray)
    {
        //青空の色を出力する
        Color color;
        color.Set(150, 200, 255);
        return color;
    }
    //光線を飛ばして色を取得する
    Color CastRay(Ray inputRay)
    {
        //すでに8回反射している場合は打ち切り
        if (inputRay.reflectCount >= MAX_REFLECTSION_SIZE)
        {
            Color black;
            black.Set(0, 0, 0);
            return black;
        }
        Color outputColor;
        //一番近い交点を求める
        double closestT = 1000;
        int objectIndex;
        bool isHit = false;
        //すべてのオブジェクトについて衝突を検索
        for (int n = 0; n < 3; n++)
        {
            //交点が存在する場合
            if (objects[n].IsHit(inputRay))
            {
                //交点がより近い場合は更新する
                if (objects[n].solveT < closestT)
                {
                    closestT = objects[n].solveT;
                    objectIndex = n;
                    isHit = true;
                }
            }
        }
        //交差がある場合は再帰的に反射させる
        if (isHit)
        {
            //反射したレイを取得
            Ray secondRay;
            //反射回数を+1する
            secondRay.reflectCount = inputRay.reflectCount + 1;
            //secondRay = objects[objectIndex].GetRay(inputRay);

            //再帰的に次のレイを飛ばす
            return objects[objectIndex].GetNormalRay(inputRay);
            //return CastRay(secondRay);
            //return Color(255, 255, 255);
        }
        //反射しない場合は背景を写す
        return BackImage(inputRay);
    }
};

int main()
{
    World world;
    world.GetImage();
    return 0;
}