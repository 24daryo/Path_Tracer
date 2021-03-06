#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <string>
#include <math.h>

using namespace std;

#define WIDTH 400        //横のピクセル数
#define HEIGHT 300       //縦のピクセル数
#define SAMPLING_SIZE 10 //サンプリング数
#define GROSS 255        //輝度の階級
#define PI 3.14159       //円周率
//#define INFINITY 1000          //無限大
#define MAX_REFLECTSION_SIZE 4 //最大反射回数

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
        /*
        Vector3 P;
        Vector3 rand(drand48(), drand48(), drand48());
        do
        {
            P = rand * 2 - Vector3(1, 1, 1);
        } while (P.Length() >= 1);
        */
        float thete = drand48() * 2 * PI;
        float u = drand48();
        float A = sqrt(1 - u);
        return Vector3(A * cos(thete), A * sin(thete), sqrt(u));
    }
    //カラー型に変換
    Color ToColor();
};

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
    //加算
    Color operator+(const Color &left) const
    {
        int R = r + left.r;
        int G = g + left.g;
        int B = b + left.b;
        if (R > GROSS)
            R = GROSS;
        if (G > GROSS)
            G = GROSS;
        if (B > GROSS)
            B = GROSS;
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
    Color operator*(const float B) const { return Color((int)(B * r), (int)(B * g), (int)(B * b)); }
    Color operator*(const Color &left) const { return Color((int)(r * left.r / GROSS), (int)(g * left.g / GROSS), (int)(b * left.b / GROSS)); }
    Color operator/(const double t) const { return Color((int)(r / t), (int)(g / t), (int)(b / t)); }
};

Color Vector3::ToColor()
{
    Vector3 v(x, y, z);
    v.Norm();
    Color color((int)((v.x + 1) * GROSS / 2), (int)((v.y + 1) * GROSS / 2), (int)((v.z + 1) * GROSS / 2));
    return color;
}

class Image
{
public:
    Color **pixel;
    int width, height;
    Image() {} //クラス宣言用
    Image(const int x, const int y)
    {
        pixel = new Color *[x];
        width = x;
        height = y;
        // 配列の各要素でintの配列を動的生成
        for (int i = 0; i < x; i++)
        {
            pixel[i] = new Color[y];
        }
    }

    void SetColor(int x, int y, Color color)
    {
        pixel[x][y] = color;
    }
    Color GetColor(int x, int y)
    {
        return pixel[x][y];
    }
};

//光線
class Ray
{
public:
    //変数
    Vector3 origin;    //中心座標
    Vector3 direction; //方向
    int reflectCount;  //反射回数
    bool inner;        //レイが物質内にいるか？

    Ray(Vector3 o, Vector3 d)
    {
        origin = o;
        direction = d;
        reflectCount = 0;
    }
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
    REFRACTION,
    BlinnPhong
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

    void Set(Color _color, float _albedo, reflectionType _type)
    {
        color = _color;
        type = _type;
        albedo = _albedo;
    }

    Ray GetRay(Vector3 P, Vector3 I, Vector3 N, bool isInner)
    {
        //printf("matGetRay");
        //拡散反射
        if (type == DIFFUSE)
        {
            //printf("%f", drand48());
            Vector3 R = N + Vector3().RandInUnitSphere();
            return Ray(P, R);
        }
        //鏡面反射
        if (type == REFLECTION)
        {
            Vector3 R = -I + N * (2 * N.Dot(I));
            return Ray(P, R);
        }
        //屈折
        if (type == REFRACTION)
        {
            //相対屈折量
            float n = 1.5;
            if (isInner)
            {
                //内部→外部
                n = n;
            }
            else
            {
                n = 1 / n;
            }
            //内積をあらかじめ計算
            float dot = I.Dot(N);
            Vector3 T = I * n + N * (n * dot - sqrt(1 - n * n * (1 - dot * dot)));
            return Ray(P, T);
        }
        /*
        //ブリンフォンs
        if (type == BlinnPhong)
        {
        }
        */
        return Ray(Vector3(0, 100, 0), Vector3(0, 1, 0)); //上空へレイを飛ばす
    }
};

//球体
class Sphere
{
public:
    //変数
    Vector3 center;         //中心座標
    float radius;           //半径
    float solveT1, solveT2; //交点の解
    Material material;      //マテリアル

    //初期化
    Sphere() {}
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
            solveT1 = (-b - sqrt(discriminant)) / a; //解1(手前側)
            solveT2 = (-b + sqrt(discriminant)) / a; //解2(奥側)
            //交点が前方に存在する場合
            if (solveT1 > 0.01)
            {
                return true;
            }
            //交点が前方で奥側に存在する場合(つまり内→外の光線)

            if (solveT2 > 0.01)
            {
                ray.inner = true;
                return true;
            }
        }
        return false;
    }
    Color GetNormalRay(Ray ray)
    {
        //反射位置Pを求める
        Vector3 P = ray.origin + ray.direction * solveT1; //P = A + tB
        //単位法線ベクトルを求める
        Vector3 N = (P - center).Norm();
        return N.ToColor();
    }
    Ray GetRay(Ray ray)
    {
        //反射位置Pを求める
        Vector3 P = ray.origin + ray.direction * solveT1; //P = A + tB
        //入射ベクトルを求める
        Vector3 I = -ray.direction.Norm();
        //法線ベクトルを求める
        Vector3 N = (P - center).Norm();
        //反射位置に関して少し表面より外側に位置をずらす(エラー回避)
        P = P + N * 0.0001;
        //出て行く光か？
        bool isInner = ray.inner;
        //素材の違いを考慮したレイを取得する
        return material.GetRay(P, I, N, isInner);
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
    Ray GetScreenRay(int i, int j)
    {
        //-0.5~0.5の乱数を取得する
        float randA = drand48() - 0.5;
        float randB = drand48() - 0.5;
        //光線の場所を求める
        //Vector3 rayPos = screenOrigin + X * i + Y * j;
        Vector3 rayPos = screenOrigin + X * (i + randA) + Y * (j + randB);
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
    Camera camera;
    //マテリアル
    Material mat1, mat2, mat3, mirror;
    //球体オブジェクト
    Sphere ball_1, ball_2, ball_3, terrain;
    Sphere objects[4];
    //背景
    Image backImage;

    //初期化
    World()
    {
        //カメラを設定
        camera.Set(Vector3(0, 0, 0), Vector3(0, 0, -1), 45); //座標、視線方向、仰角
        //マテリアルを設定
        mat1.Set(Color(0, 200, 255), 0.5, DIFFUSE);
        mat2.Set(Color(255, 100, 200), 0.5, DIFFUSE);
        mat3.Set(Color(100, 255, 100), 0.5, DIFFUSE);
        mirror.Set(Color(255, 255, 150), 0.5, REFLECTION);
        //オブジェクトを設定
        ball_1.Set(Vector3(2, 0, -3), 1, mat1);
        ball_2.Set(Vector3(0, 0, -3), 1, mat2);
        ball_3.Set(Vector3(-2, 0, -3), 1, mat3);
        terrain.Set(Vector3(2, -501, 2), 500, mirror);
        //オブジェクトを登録
        objects[0] = ball_1;
        objects[1] = ball_2;
        objects[2] = ball_3;
        objects[3] = terrain;
        //背景を登録
        backImage = ReadPPM();
    }
    //画像生成
    void GetImage()
    {
        //printf("tesuto");
        /*============================*/
        /*   カラー画像の書き込み     */
        /*============================*/
        FILE *fp;
        fp = fopen("./output1.ppm", "wb");     //■ write binaryモードで画像ファイルを開く
        fprintf(fp, "P3\n");                   //■ ファイルの識別符号を書き込む
        fprintf(fp, "%d %d\n", WIDTH, HEIGHT); //■ 画像サイズを書き込む
        fprintf(fp, "%d\n", GROSS);            //■ 最大輝度値を書き込む

        //■ 画像データの書き込み
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                //カラーを格納する変数を作成
                int red = 0, green = 0, blue = 0;
                //レイを飛ばす
                for (int n = 0; n < SAMPLING_SIZE; n++)
                {
                    //printf("完了度：(%d, %d, %d)\n", y, x, n);
                    //カメラからの光線を取得する
                    Ray cameraRay = camera.GetScreenRay(x, y);
                    //光線を飛ばして色を取得し加算
                    //Color sum = CastRay(cameraRay);
                    //Color sum = GetNormal(cameraRay);
                    //Color sum = GetDepth(cameraRay);
                    //Color sum = GetColor(cameraRay);
                    //Color sum = GetSecond(cameraRay);
                    Color sum = GetOutline(cameraRay);
                    red += sum.r;
                    green += sum.g;
                    blue += sum.b;
                }
                //すること
                //被写界深度深度を求める
                //ピンボケを作る
                //法線ベクトルを作る
                //サンプリングの平均を取得
                red = (int)(red / SAMPLING_SIZE);
                green = (int)(green / SAMPLING_SIZE);
                blue = (int)(blue / SAMPLING_SIZE);
                //ガンマ変換
                double gamma = (double)(1 / 2.2);
                red = (int)(GROSS * pow(((double)red / GROSS), gamma));
                green = (int)(GROSS * pow(((double)green / GROSS), gamma));
                blue = (int)(GROSS * pow(((double)blue / GROSS), gamma));
                //取得した色を書き込む
                fprintf(fp, "%d %d %d ", red, green, blue);
            }
        }
        fclose(fp);
    }

private:
    //世界の背景を取得
    Color BackImage(Ray ray)
    {
        ray.direction.Norm();
        float x = (float)ray.direction.x;
        float y = (float)ray.direction.y;
        float z = (float)ray.direction.z;
        //円柱座標変換
        float theta; //(-Pi/2)~(Pi/2)
        if ((x >= 0) && (z >= 0))
        {
            //第1象限
            theta = (float)atan(z / x);
        }
        if ((x < 0) && (z >= 0))
        {
            //第2象限
            theta = (float)atan(z / x) + PI;
        }
        if ((x < 0) && (z < 0))
        {
            //第3象限
            theta = (float)atan(z / x) + PI;
        }
        if ((x >= 0) && (z < 0))
        {
            //第4象限
            theta = (float)atan(z / x) + 2 * PI;
        }
        float height = sqrt(1 - (x * x + z * z));
        if (y < 0)
        {
            height = -height;
        }
        //printf("%f\n", theta);
        //光線をスクリーンに変換する
        int u, v;
        u = (int)((255 - 1) * (theta / (2.0 * PI)));
        v = (int)((255 - 1) * ((height + 1.0) / 2.0));
        //printf("%d\n", u);
        //画像の色を取得する
        //return backImage.GetColor(u, v);
        return Color(u, v, v);
        //青空の色を出力する
        //return Color(150, 200, 255);
    }
    //光線を飛ばして色を取得する
    Color GetNormal(Ray inputRay)
    { //一番近い交点を求める
        double closestT = 1000;
        int objectIndex;
        bool isHit = false;
        //すべてのオブジェクトについて衝突を検索
        for (int n = 0; n < 4; n++)
        {
            //交点が存在する場合
            if (objects[n].IsHit(inputRay))
            {
                //交点がより近い場合は更新する
                if (objects[n].solveT1 < closestT)
                {
                    closestT = objects[n].solveT1;
                    objectIndex = n;
                    isHit = true;
                }
            }
        }
        //交差がある場合は再帰的に反射させる
        if (isHit)
        {
            //反射した物体のマテリアルを取得
            Material material = objects[objectIndex].material;
            //反射したレイを取得
            Ray secondRay = objects[objectIndex].GetRay(inputRay);
            //反射回数を+1する
            secondRay.reflectCount = inputRay.reflectCount + 1;
            //再帰的に次のレイを飛ばす
            return objects[objectIndex].GetNormalRay(inputRay); //法線ベクトルを求める
            //return CastRay(secondRay) * material.albedo * material.color;
            //return Color(255, 255, 255); //ぶつかると真っ白
        }
        //反射しない場合は背景を写す
        return BackImage(inputRay);
    }
    //光線を飛ばして色を取得する
    Color GetDepth(Ray inputRay)
    {
        //一番近い交点を求める
        double closestT = 1000;
        int objectIndex;
        bool isHit = false;
        //すべてのオブジェクトについて衝突を検索
        for (int n = 0; n < 4; n++)
        {
            //交点が存在する場合
            if (objects[n].IsHit(inputRay))
            {
                //交点がより近い場合は更新する
                if (objects[n].solveT1 < closestT)
                {
                    closestT = objects[n].solveT1;
                    objectIndex = n;
                    isHit = true;
                }
            }
        }
        //交差がある場合は再帰的に反射させる
        if (isHit)
        {
            //解を取得
            double solve = objects[objectIndex].solveT1;
            //基準を６として算出
            int depthGross = (int)(((-1.0 / 4.0) * solve + 1.0) * 255);
            return Color(depthGross, depthGross, depthGross); //ぶつかると真っ白
        }
        //反射しない場合は背景を写す
        return Color(0, 0, 0);
    }
    //光線を飛ばして色を取得する
    Color GetColor(Ray inputRay)
    {
        //一番近い交点を求める
        double closestT = 1000;
        int objectIndex;
        bool isHit = false;
        //すべてのオブジェクトについて衝突を検索
        for (int n = 0; n < 4; n++)
        {
            //交点が存在する場合
            if (objects[n].IsHit(inputRay))
            {
                //交点がより近い場合は更新する
                if (objects[n].solveT1 < closestT)
                {
                    closestT = objects[n].solveT1;
                    objectIndex = n;
                    isHit = true;
                }
            }
        }
        //交差がある場合は再帰的に反射させる
        if (isHit)
        {
            //反射した物体のマテリアルを取得
            Material material = objects[objectIndex].material;
            //反射したレイを取得
            Ray secondRay = objects[objectIndex].GetRay(inputRay);
            //反射回数を+1する
            secondRay.reflectCount = inputRay.reflectCount + 1;
            //再帰的に次のレイを飛ばす
            return material.color; //法線ベクトルを求める
            //return CastRay(secondRay) * material.albedo * material.color;
            //return Color(255, 255, 255); //ぶつかると真っ白
        }
        //反射しない場合は背景を写す
        return BackImage(inputRay);
    }
    //光線を飛ばして色を取得する
    Color GetOutline(Ray inputRay)
    {
        //一番近い交点を求める
        double closestT = 1000;
        int objectIndex;
        bool isHit = false;
        //すべてのオブジェクトについて衝突を検索
        for (int n = 0; n < 4; n++)
        {
            //交点が存在する場合
            if (objects[n].IsHit(inputRay))
            {
                //交点がより近い場合は更新する
                if (objects[n].solveT1 < closestT)
                {
                    closestT = objects[n].solveT1;
                    objectIndex = n;
                    isHit = true;
                }
            }
        }

        //交差がある場合は再帰的に反射させる
        if (isHit)
        {
            //反射位置Pを求める
            Vector3 P = inputRay.origin + inputRay.direction * objects[objectIndex].solveT1; //P = A + tB
            //入射ベクトルを求める
            Vector3 I = -inputRay.direction.Norm();
            //法線ベクトルを求める
            Vector3 N = (P - objects[objectIndex].center).Norm();
            if (I.Dot(N) < 0.1 / sqrt(sqrt(objects[objectIndex].radius)))
            {
                return Color(255, 255, 255);
            }
        }
        //反射しない場合は背景を写す
        return Color(0, 0, 0);
    }
    //光線を飛ばして色を取得する
    Color GetSecond(Ray inputRay)
    {
        //すでに8回反射している場合は打ち切り
        if (inputRay.reflectCount >= 4)
        {
            Color black(0, 0, 0);
            return black;
        }
        //一番近い交点を求める
        double closestT = 1000;
        int objectIndex;
        bool isHit = false;
        //すべてのオブジェクトについて衝突を検索
        for (int n = 0; n < 4; n++)
        {
            //交点が存在する場合
            if (objects[n].IsHit(inputRay))
            {
                //交点がより近い場合は更新する
                if (objects[n].solveT1 < closestT)
                {
                    closestT = objects[n].solveT1;
                    objectIndex = n;
                    isHit = true;
                }
            }
        }
        //交差がある場合は再帰的に反射させる
        if (isHit)
        {
            //反射した物体のマテリアルを取得
            Material material = objects[objectIndex].material;
            //反射したレイを取得
            Ray secondRay = objects[objectIndex].GetRay(inputRay);
            //反射回数を+1する
            secondRay.reflectCount = inputRay.reflectCount + 1;
            //再帰的に次のレイを飛ばす
            //return objects[objectIndex].GetNormalRay(inputRay); //法線ベクトルを求める
            if (inputRay.reflectCount == 0)
            {
                return CastRay(secondRay) - (material.color * material.albedo);
            }
            else
            {
                return CastRay(secondRay) * material.albedo * material.color;
            }

            //return Color(255, 255, 255); //ぶつかると真っ白
        }
        //反射しない場合は背景を写す
        return BackImage(inputRay);
    }
    //光線を飛ばして色を取得する
    Color CastRay(Ray inputRay)
    {
        //すでに8回反射している場合は打ち切り
        if (inputRay.reflectCount >= MAX_REFLECTSION_SIZE)
        {
            Color black(0, 0, 0);
            return black;
        }
        //一番近い交点を求める
        double closestT = 1000;
        int objectIndex;
        bool isHit = false;
        //すべてのオブジェクトについて衝突を検索
        for (int n = 0; n < 4; n++)
        {
            //交点が存在する場合
            if (objects[n].IsHit(inputRay))
            {
                //交点がより近い場合は更新する
                if (objects[n].solveT1 < closestT)
                {
                    closestT = objects[n].solveT1;
                    objectIndex = n;
                    isHit = true;
                }
            }
        }
        //交差がある場合は再帰的に反射させる
        if (isHit)
        {
            //反射した物体のマテリアルを取得
            Material material = objects[objectIndex].material;
            //反射したレイを取得
            Ray secondRay = objects[objectIndex].GetRay(inputRay);
            //反射回数を+1する
            secondRay.reflectCount = inputRay.reflectCount + 1;
            //再帰的に次のレイを飛ばす
            //return objects[objectIndex].GetNormalRay(inputRay); //法線ベクトルを求める
            return CastRay(secondRay) * material.albedo * material.color;
            //return Color(255, 255, 255); //ぶつかると真っ白
        }
        //反射しない場合は背景を写す
        return BackImage(inputRay);
    }
    Image ReadPPM()
    {
        int i, j;       //■ ループ用変数
        char buff[128]; //■ ヘッダ抽出用
        int x, y;       //■ 画像ファイルのヘッダ操作用変数
        FILE *fp;       //■ ファイルポインタ

        /*============================*/
        /*        画像入力部           */
        /*============================*/
        fp = fopen("./backImage.ppm", "rb"); //■ read binaryモードで画像ファイルを開く
        //■ ファイルの識別符号を読み込み
        fgets(buff, 128, fp);
        printf("%s", buff);
        //■ 画像サイズの読み込み
        int imageW, imageH;
        //ピクセル横
        fscanf(fp, "%s", buff);
        imageW = atoi(buff);
        printf("%s\n", buff);
        //ピクセル縦
        fscanf(fp, "%s", buff);
        imageH = atoi(buff);
        printf("%s\n", buff);
        //■ 最大輝度値の読み込み
        fgets(buff, 128, fp);
        printf("%s", buff);

        //■ 画像データの読み込み
        unsigned char Pix[imageW][imageH * 3]; //■ バイナリ画像データ格納用
        for (j = 0; j < imageW; j++)
            fread(Pix[j], sizeof(unsigned char), imageH * 3, fp);
        fclose(fp);

        /*============================*/
        /*       画像処理部            */
        /*============================*/
        Image image(imageW, imageH);
        for (j = 0; j < imageW; j++)
        {
            for (i = 0; i < imageH; i++)
            {
                //printf("(x,y)=(%d %d)のR値は%dです\n", j, i, Pix[j][3 * i]);
                //printf("(x,y)=(%d %d)のG値は%dです\n", j, i, Pix[j][3 * i + 1]);
                //printf("(x,y)=(%d %d)のB値は%dです\n", j, i, Pix[j][3 * i + 2]);
                Color tmp = Color(Pix[j][3 * i], Pix[j][3 * i + 1], Pix[j][3 * i + 2]);
                image.SetColor(j, i, tmp);
            }
        }
        return image;
    }
};

int main()
{
    World world;
    world.GetImage();
    return 0;
}