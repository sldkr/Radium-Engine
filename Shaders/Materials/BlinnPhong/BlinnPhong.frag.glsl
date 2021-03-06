//This is for a preview of the shader composition, but in time we must use more specific Light Shader
#include "DefaultLight.glsl"
#include "BlinnPhongMaterial.glsl"

uniform sampler2D uShadowMap;

out vec4 fragColor;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_texcoord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_viewVector;
layout (location = 5) in vec3 in_lightVector;

void main() {
    if ( toDiscard(material, in_texcoord.xy) )
        discard;

	vec3 binormal = normalize(cross(in_normal, in_tangent));

	vec3 normalLocal 	= getNormal(material, in_texcoord.xy, in_normal, in_tangent, binormal);
	vec3 binormalLocal 	= normalize(cross(normalLocal, in_tangent));
	vec3 tangentLocal 	= normalize(cross(binormalLocal, normalLocal));

	vec3 materialColor 	= computeMaterialInternal(material, in_texcoord.xy, in_lightVector, in_viewVector,
	 											  normalLocal, tangentLocal, binormalLocal);

	vec3 attenuation 	= lightContributionFrom(light, in_position);

    fragColor = vec4(materialColor * attenuation, 1.0);
}