#version 330

in vec3 normalInterp;
in vec3 vertPos;
in vec3 lightCam;
in vec3 worldPos;
in vec2 TexCoords;

uniform mat4 view;
uniform bool simpleColor;
uniform sampler2D colorTexture;

vec3 ambientColor = vec3(0.2, 0.1, 0.0);
vec3 diffuseColor = vec3(73 / 255, 56 / 255, 28 / 255);
vec3 specColor    = vec3(1.0, 1.0, 1.0);

out vec4 fragmentColor;

void main()
{
	vec3 color = texture(colorTexture,TexCoords).rgb;
    if(!simpleColor)
    {
        vec3 normal   = normalize(normalInterp);
        vec3 lightDir = normalize(lightCam - vertPos);

        float lambertian = max(dot(lightDir, normal), 0.0);
        float specular   = 0.0;

        if(lambertian > 0.0)
        {
            vec3 viewDir = normalize(-vertPos);

            // this is blinn phong
            vec3  halfDir   = normalize(lightDir + viewDir);
            float specAngle = max(dot(halfDir, normal), 0.0);
            specular        = pow(specAngle, 16.0);

            vec3 reflectDir = reflect(-lightDir, normal);
            specAngle       = max(dot(reflectDir, viewDir), 0.0);
            // note that the exponent is different here
            specular = 0;
        }

        fragmentColor   = vec4(color + lambertian * diffuseColor + specular * specColor, 1.0);
        float distToCam = (-(view * vec4(vertPos, 1.0)).z - 0.1) / (100000.f - .1f);
        fragmentColor   = mix(fragmentColor, vec4(0.1, 0.1, 0.1, 1.0), distToCam);
    }
    else
        fragmentColor = vec4(1, 0, 0, 1);
}
