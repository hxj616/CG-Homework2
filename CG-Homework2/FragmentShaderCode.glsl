#version 430

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightDir;     // ����ⷽ�򣨼����ѹ�һ����
uniform vec3 lightColor;    // ����ɫ��RGB��ʽ��
uniform float lightIntensity; // ��ǿ��ϵ��
uniform vec3 viewPos;       // �����λ��

void main()
{
    // ��׼������
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDirection = normalize(-lightDir); // ת���ⷽ��ΪƬ��ָ���Դ

    // �������գ���ǿ�ȹ�����
    vec3 ambient = lightIntensity * 0.2 * lightColor;

    // ���������
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = lightIntensity * diff * lightColor;

    // ���淴�䣨Blinn-Phong�Ż���
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = lightIntensity * spec * lightColor;

    // �������
    vec3 texColor = texture(texture1, TexCoord).rgb;

    // ������ɫ�ϳ�
    vec3 result = (ambient + diffuse) * texColor +  // ����/������Ӱ��������ɫ
        specular * 0.5;                   // ����������ǿ
    FragColor = vec4(result, 1.0);
}