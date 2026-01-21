#version 100

precision highp float;
precision highp int;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 uViewportSize;

#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     8.0

vec4 applyFXAA(vec2 fragCoord, sampler2D tex)
{
    vec4 color;
    vec2 inverseVP = vec2(1.0 / uViewportSize.x, 1.0 / uViewportSize.y);
    
    vec4 texelNW = texture2D(tex, (fragCoord + vec2(-1.0, -1.0)) * inverseVP);
    vec4 texelNE = texture2D(tex, (fragCoord + vec2(1.0, -1.0)) * inverseVP);
    vec4 texelSW = texture2D(tex, (fragCoord + vec2(-1.0, 1.0)) * inverseVP);
    vec4 texelSE = texture2D(tex, (fragCoord + vec2(1.0, 1.0)) * inverseVP);
    vec4 texelM  = texture2D(tex, fragCoord * inverseVP);
    
    vec3 rgbNW = texelNW.rgb;
    vec3 rgbNE = texelNE.rgb;
    vec3 rgbSW = texelSW.rgb;
    vec3 rgbSE = texelSE.rgb;
    vec3 rgbM  = texelM.rgb;
    
    float alphaNW = texelNW.a;
    float alphaNE = texelNE.a;
    float alphaSW = texelSW.a;
    float alphaSE = texelSE.a;
    float alphaM  = texelM.a;
    
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM, luma);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
              dir * rcpDirMin)) * inverseVP;
      
    vec3 rgbA = 0.5 * (
        texture2D(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).rgb +
        texture2D(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).rgb);
    
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture2D(tex, fragCoord * inverseVP + dir * -0.5).rgb +
        texture2D(tex, fragCoord * inverseVP + dir * 0.5).rgb);

    float alphaA = 0.5 * (
        texture2D(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).a +
        texture2D(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).a);
    
    float alphaB = alphaA * 0.5 + 0.25 * (
        texture2D(tex, fragCoord * inverseVP + dir * -0.5).a +
        texture2D(tex, fragCoord * inverseVP + dir * 0.5).a);

    float lumaB = dot(rgbB, luma);
    
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, alphaA);
    else
        color = vec4(rgbB, alphaB);
    
    return color;
}

void main()
{
    vec2 fragCoord = fragTexCoord * uViewportSize;
    vec4 fxaaColor = applyFXAA(fragCoord, texture0);
    gl_FragColor = fxaaColor * colDiffuse;
    gl_FragColor *= fragColor;
}