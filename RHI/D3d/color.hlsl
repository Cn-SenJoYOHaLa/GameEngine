cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj; 
};

// 顶点输入结构
struct VertexIn
{
    float3 PosL  : POSITION;  // 位置，对应输入签名 POSITION
    float4 Color : COLOR;     // 颜色，对应输入签名 COLOR
};

// 顶点输出结构
struct VertexOut
{
    float4 PosH  : SV_POSITION; // 系统（SC_ sys value）顶点位置信息输出
    float4 Color : COLOR; // 自定输出语义 COLOR
};

//-----------------------------------
// 顶点着色器入口函数
//-----------------------------------
VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // 将顶点坐标变换到齐次裁剪空间
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

    // 直接把顶点颜色传给像素shader
    vout.Color = vin.Color;

    return vout;
}

struct VertexOut
{
    float4 PosH  : SV_POSITION; // 系统（SC_ sys value）顶点位置信息输出
    float4 Color : COLOR; // 自定输出语义 COLOR
};

//-----------------------------------
// 像素着色器入口函数，SV_TARGET表示返回值渲染目标格式
//-----------------------------------
float4 PS(VertexOut pin) : SV_Target
{
    // 直接返回插值颜色数据
    return pin.Color;
}