#version 430

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightDir;     // 定向光方向（假设已归一化）
uniform vec3 lightColor;    // 光颜色（RGB格式）
uniform float lightIntensity; // 光强度系数
uniform vec3 viewPos;       // 摄像机位置

void main()
{
    // 标准化向量
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDirection = normalize(-lightDir); // 转换光方向为片段指向光源

    // 环境光照（与强度关联）
    vec3 ambient = lightIntensity * 0.2 * lightColor;

    // 漫反射计算
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = lightIntensity * diff * lightColor;

    // 镜面反射（Blinn-Phong优化）
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = lightIntensity * spec * lightColor;

    // 纹理采样
    vec3 texColor = texture(texture1, TexCoord).rgb;

    // 最终颜色合成
    vec3 result = (ambient + diffuse) * texColor +  // 环境/漫反射影响纹理颜色
        specular * 0.5;                   // 镜面光独立增强
    FragColor = vec4(result, 1.0);
}