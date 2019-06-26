#include "GSH_OpenGL.h"
#include <assert.h>
#include <sstream>

#ifdef GLES_COMPATIBILITY
#define GLSL_VERSION "#version 300 es"
#else
//#define GLSL_VERSION "#version 150"
#define GLSL_VERSION "#version 420"
#endif

enum FRAGMENT_SHADER_ORDERING_MODE
{
	FRAGMENT_SHADER_ORDERING_NONE = 0,
	FRAGMENT_SHADER_ORDERING_INTEL = 1,
	FRAGMENT_SHADER_ORDERING_NV = 2,
	FRAGMENT_SHADER_ORDERING_ARB = 3,
};

static const char* s_andFunction =
    "float and(int a, int b)\r\n"
    "{\r\n"
    "	int r = 0;\r\n"
    "	int ha, hb;\r\n"
    "	\r\n"
    "	int m = int(min(float(a), float(b)));\r\n"
    "	\r\n"
    "	for(int k = 1; k <= m; k *= 2)\r\n"
    "	{\r\n"
    "		ha = a / 2;\r\n"
    "		hb = b / 2;\r\n"
    "		if(((a - ha * 2) != 0) && ((b - hb * 2) != 0))\r\n"
    "		{\r\n"
    "			r += k;\r\n"
    "		}\r\n"
    "		a = ha;\r\n"
    "		b = hb;\r\n"
    "	}\r\n"
    "	\r\n"
    "	return float(r);\r\n"
    "}\r\n";

static const char* s_orFunction =
    "float or(int a, int b)\r\n"
    "{\r\n"
    "	int r = 0;\r\n"
    "	int ha, hb;\r\n"
    "	\r\n"
    "	int m = int(max(float(a), float(b)));\r\n"
    "	\r\n"
    "	for(int k = 1; k <= m; k *= 2)\r\n"
    "	{\r\n"
    "		ha = a / 2;\r\n"
    "		hb = b / 2;\r\n"
    "		if(((a - ha * 2) != 0) || ((b - hb * 2) != 0))\r\n"
    "		{\r\n"
    "			r += k;\r\n"
    "		}\r\n"
    "		a = ha;\r\n"
    "		b = hb;\r\n"
    "	}\r\n"
    "	\r\n"
    "	return float(r);\r\n"
    "}\r\n";

Framework::OpenGl::ProgramPtr CGSH_OpenGL::GenerateShader(const SHADERCAPS& caps)
{
	auto vertexShader = GenerateVertexShader(caps);
	auto fragmentShader = GenerateFragmentShader(caps);

	auto result = std::make_shared<Framework::OpenGl::CProgram>();

	result->AttachShader(vertexShader);
	result->AttachShader(fragmentShader);

	glBindAttribLocation(*result, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::POSITION), "a_position");
	glBindAttribLocation(*result, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::DEPTH), "a_depth");
	glBindAttribLocation(*result, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::COLOR), "a_color");
	glBindAttribLocation(*result, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::TEXCOORD), "a_texCoord");
	glBindAttribLocation(*result, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::FOG), "a_fog");

	FRAMEWORK_MAYBE_UNUSED bool linkResult = result->Link();
	assert(linkResult);

	CHECKGLERROR();

	return result;
}

Framework::OpenGl::CShader CGSH_OpenGL::GenerateVertexShader(const SHADERCAPS& caps)
{
	std::stringstream shaderBuilder;
	shaderBuilder << GLSL_VERSION << std::endl;

	shaderBuilder << "layout(std140) uniform VertexParams" << std::endl;
	shaderBuilder << "{" << std::endl;
	shaderBuilder << "	mat4 g_projMatrix;" << std::endl;
	shaderBuilder << "	mat4 g_texMatrix;" << std::endl;
	shaderBuilder << "};" << std::endl;

	shaderBuilder << "in vec2 a_position;" << std::endl;
	shaderBuilder << "in uint a_depth;" << std::endl;
	shaderBuilder << "in vec4 a_color;" << std::endl;
	shaderBuilder << "in vec3 a_texCoord;" << std::endl;

	shaderBuilder << "out float v_depth;" << std::endl;
	shaderBuilder << "out vec4 v_color;" << std::endl;
	shaderBuilder << "out vec3 v_texCoord;" << std::endl;
	if(caps.hasFog)
	{
		shaderBuilder << "in float a_fog;" << std::endl;
		shaderBuilder << "out float v_fog;" << std::endl;
	}

	shaderBuilder << "void main()" << std::endl;
	shaderBuilder << "{" << std::endl;
	shaderBuilder << "	vec4 texCoord = g_texMatrix * vec4(a_texCoord, 1);" << std::endl;
	shaderBuilder << "	v_depth = float(a_depth) / 4294967296.0;" << std::endl;
	shaderBuilder << "	v_color = a_color;" << std::endl;
	shaderBuilder << "	v_texCoord = texCoord.xyz;" << std::endl;
	if(caps.hasFog)
	{
		shaderBuilder << "	v_fog = a_fog;" << std::endl;
	}
	shaderBuilder << "	gl_Position = g_projMatrix * vec4(a_position, 0, 1);" << std::endl;
	shaderBuilder << "}" << std::endl;

	auto shaderSource = shaderBuilder.str();

	Framework::OpenGl::CShader result(GL_VERTEX_SHADER);
	result.SetSource(shaderSource.c_str(), shaderSource.size());
	FRAMEWORK_MAYBE_UNUSED bool compilationResult = result.Compile();
	assert(compilationResult);

	CHECKGLERROR();

	return result;
}

Framework::OpenGl::CShader CGSH_OpenGL::GenerateFragmentShader(const SHADERCAPS& caps)
{
	std::stringstream shaderBuilder;

	auto orderingMode = FRAGMENT_SHADER_ORDERING_NONE;

	if(GLEW_ARB_fragment_shader_interlock)
	{
		orderingMode = FRAGMENT_SHADER_ORDERING_ARB;
	}
	//else if(GLEW_NV_fragment_shader_interlock)
	//{
	//	orderingMode = FRAGMENT_SHADER_ORDERING_NV;
	//}
	else if(GLEW_INTEL_fragment_shader_ordering)
	{
		orderingMode = FRAGMENT_SHADER_ORDERING_INTEL;
	}

	shaderBuilder << GLSL_VERSION << std::endl;

	switch(orderingMode)
	{
	case FRAGMENT_SHADER_ORDERING_ARB:
		shaderBuilder << "#extension GL_ARB_fragment_shader_interlock : enable" << std::endl;
		shaderBuilder << "layout(pixel_interlock_ordered) in;" << std::endl;
		break;
	case FRAGMENT_SHADER_ORDERING_INTEL:
		shaderBuilder << "#extension GL_INTEL_fragment_shader_ordering : enable" << std::endl;
		break;
	}

	shaderBuilder << "precision mediump float;" << std::endl;

	shaderBuilder << "in highp float v_depth;" << std::endl;
	shaderBuilder << "in vec4 v_color;" << std::endl;
	shaderBuilder << "in highp vec3 v_texCoord;" << std::endl;
	if(caps.hasFog)
	{
		shaderBuilder << "in float v_fog;" << std::endl;
	}

	shaderBuilder << "out vec4 fragColor;" << std::endl;

	shaderBuilder << "uniform sampler2D g_texture;" << std::endl;
	shaderBuilder << "uniform sampler2D g_palette;" << std::endl;

	shaderBuilder << "uniform layout(rgba8) image2D g_framebuffer;" << std::endl;
	shaderBuilder << "uniform layout(r32ui) uimage2D g_depthbuffer;" << std::endl;

	shaderBuilder << "layout(std140) uniform FragmentParams" << std::endl;
	shaderBuilder << "{" << std::endl;
	shaderBuilder << "	vec2 g_textureSize;" << std::endl;
	shaderBuilder << "	vec2 g_texelSize;" << std::endl;
	shaderBuilder << "	vec2 g_clampMin;" << std::endl;
	shaderBuilder << "	vec2 g_clampMax;" << std::endl;
	shaderBuilder << "	float g_texA0;" << std::endl;
	shaderBuilder << "	float g_texA1;" << std::endl;
	shaderBuilder << "	uint g_alphaRef;" << std::endl;
	shaderBuilder << "	uint g_depthMask;" << std::endl;
	shaderBuilder << "	vec3 g_fogColor;" << std::endl;
	shaderBuilder << "	uint g_alphaFix;" << std::endl;
	shaderBuilder << "	uint g_colorMask;" << std::endl;
	shaderBuilder << "};" << std::endl;

	if(caps.texClampS == TEXTURE_CLAMP_MODE_REGION_REPEAT || caps.texClampT == TEXTURE_CLAMP_MODE_REGION_REPEAT)
	{
		shaderBuilder << s_andFunction << std::endl;
		shaderBuilder << s_orFunction << std::endl;
	}

	shaderBuilder << "float combineColors(float a, float b)" << std::endl;
	shaderBuilder << "{" << std::endl;
	shaderBuilder << "	uint aInt = uint(a * 255.0);" << std::endl;
	shaderBuilder << "	uint bInt = uint(b * 255.0);" << std::endl;
	shaderBuilder << "	uint result = min((aInt * bInt) >> 7, 255u);" << std::endl;
	shaderBuilder << "	return float(result) / 255.0;" << std::endl;
	shaderBuilder << "}" << std::endl;

	shaderBuilder << "vec4 expandAlpha(vec4 inputColor)" << std::endl;
	shaderBuilder << "{" << std::endl;
	if(caps.texUseAlphaExpansion)
	{
		shaderBuilder << "	float alpha = mix(g_texA0, g_texA1, inputColor.a);" << std::endl;
		if(caps.texBlackIsTransparent)
		{
			shaderBuilder << "	float black = inputColor.r + inputColor.g + inputColor.b;" << std::endl;
			shaderBuilder << "	if(black == 0.0) alpha = 0.0;" << std::endl;
		}
		shaderBuilder << "	return vec4(inputColor.rgb, alpha);" << std::endl;
	}
	else
	{
		shaderBuilder << "	return inputColor;" << std::endl;
	}
	shaderBuilder << "}" << std::endl;

	shaderBuilder << "void main()" << std::endl;
	shaderBuilder << "{" << std::endl;

	shaderBuilder << "	uint depth = uint(v_depth * 4294967296.0);" << std::endl;

	shaderBuilder << "	bool depthTestFail = false;" << std::endl;
	shaderBuilder << "	bool alphaTestFail = false;" << std::endl;

	shaderBuilder << "	highp vec3 texCoord = v_texCoord;" << std::endl;
	shaderBuilder << "	texCoord.st /= texCoord.p;" << std::endl;

	if((caps.texClampS != TEXTURE_CLAMP_MODE_STD) || (caps.texClampT != TEXTURE_CLAMP_MODE_STD))
	{
		shaderBuilder << "	texCoord.st *= g_textureSize.st;" << std::endl;
		shaderBuilder << GenerateTexCoordClampingSection(static_cast<TEXTURE_CLAMP_MODE>(caps.texClampS), "s");
		shaderBuilder << GenerateTexCoordClampingSection(static_cast<TEXTURE_CLAMP_MODE>(caps.texClampT), "t");
		shaderBuilder << "	texCoord.st /= g_textureSize.st;" << std::endl;
	}

	shaderBuilder << "	vec4 textureColor = vec4(1, 1, 1, 1);" << std::endl;
	if(caps.isIndexedTextureSource())
	{
		if(!caps.texBilinearFilter)
		{
			shaderBuilder << "	float colorIndex = texture(g_texture, texCoord.st).r * 255.0;" << std::endl;
			if(caps.texSourceMode == TEXTURE_SOURCE_MODE_IDX4)
			{
				shaderBuilder << "	float paletteTexelBias = 0.5 / 16.0;" << std::endl;
				shaderBuilder << "	textureColor = expandAlpha(texture(g_palette, vec2(colorIndex / 16.0 + paletteTexelBias, 0)));" << std::endl;
			}
			else if(caps.texSourceMode == TEXTURE_SOURCE_MODE_IDX8)
			{
				shaderBuilder << "	float paletteTexelBias = 0.5 / 256.0;" << std::endl;
				shaderBuilder << "	textureColor = expandAlpha(texture(g_palette, vec2(colorIndex / 256.0 + paletteTexelBias, 0)));" << std::endl;
			}
		}
		else
		{
			shaderBuilder << "	float tlIdx = texture(g_texture, texCoord.st                                     ).r * 255.0;" << std::endl;
			shaderBuilder << "	float trIdx = texture(g_texture, texCoord.st + vec2(g_texelSize.x, 0)            ).r * 255.0;" << std::endl;
			shaderBuilder << "	float blIdx = texture(g_texture, texCoord.st + vec2(0, g_texelSize.y)            ).r * 255.0;" << std::endl;
			shaderBuilder << "	float brIdx = texture(g_texture, texCoord.st + vec2(g_texelSize.x, g_texelSize.y)).r * 255.0;" << std::endl;

			if(caps.texSourceMode == TEXTURE_SOURCE_MODE_IDX4)
			{
				shaderBuilder << "	float paletteTexelBias = 0.5 / 16.0;" << std::endl;
				shaderBuilder << "	vec4 tl = expandAlpha(texture(g_palette, vec2(tlIdx / 16.0 + paletteTexelBias, 0)));" << std::endl;
				shaderBuilder << "	vec4 tr = expandAlpha(texture(g_palette, vec2(trIdx / 16.0 + paletteTexelBias, 0)));" << std::endl;
				shaderBuilder << "	vec4 bl = expandAlpha(texture(g_palette, vec2(blIdx / 16.0 + paletteTexelBias, 0)));" << std::endl;
				shaderBuilder << "	vec4 br = expandAlpha(texture(g_palette, vec2(brIdx / 16.0 + paletteTexelBias, 0)));" << std::endl;
			}
			else if(caps.texSourceMode == TEXTURE_SOURCE_MODE_IDX8)
			{
				shaderBuilder << "	float paletteTexelBias = 0.5 / 256.0;" << std::endl;
				shaderBuilder << "	vec4 tl = expandAlpha(texture(g_palette, vec2(tlIdx / 256.0 + paletteTexelBias, 0)));" << std::endl;
				shaderBuilder << "	vec4 tr = expandAlpha(texture(g_palette, vec2(trIdx / 256.0 + paletteTexelBias, 0)));" << std::endl;
				shaderBuilder << "	vec4 bl = expandAlpha(texture(g_palette, vec2(blIdx / 256.0 + paletteTexelBias, 0)));" << std::endl;
				shaderBuilder << "	vec4 br = expandAlpha(texture(g_palette, vec2(brIdx / 256.0 + paletteTexelBias, 0)));" << std::endl;
			}

			shaderBuilder << "	highp vec2 f = fract(texCoord.st * g_textureSize);" << std::endl;
			shaderBuilder << "	vec4 tA = mix(tl, tr, f.x);" << std::endl;
			shaderBuilder << "	vec4 tB = mix(bl, br, f.x);" << std::endl;
			shaderBuilder << "	textureColor = mix(tA, tB, f.y);" << std::endl;
		}
	}
	else if(caps.texSourceMode == TEXTURE_SOURCE_MODE_STD)
	{
		shaderBuilder << "	textureColor = expandAlpha(texture(g_texture, texCoord.st));" << std::endl;
	}

	if(caps.texSourceMode != TEXTURE_SOURCE_MODE_NONE)
	{
		if(!caps.texHasAlpha)
		{
			shaderBuilder << "	textureColor.a = 1.0;" << std::endl;
		}

		switch(caps.texFunction)
		{
		case TEX0_FUNCTION_MODULATE:
			shaderBuilder << "	textureColor.rgb = clamp(textureColor.rgb * v_color.rgb * 2.0, 0.0, 1.0);" << std::endl;
			if(!caps.texHasAlpha)
			{
				shaderBuilder << "	textureColor.a = v_color.a;" << std::endl;
			}
			else
			{
				shaderBuilder << "	textureColor.a = combineColors(textureColor.a, v_color.a);" << std::endl;
			}
			break;
		case TEX0_FUNCTION_DECAL:
			break;
		case TEX0_FUNCTION_HIGHLIGHT:
			shaderBuilder << "	textureColor.rgb = clamp(textureColor.rgb * v_color.rgb * 2.0, 0.0, 1.0) + v_color.aaa;" << std::endl;
			if(!caps.texHasAlpha)
			{
				shaderBuilder << "	textureColor.a = v_color.a;" << std::endl;
			}
			else
			{
				shaderBuilder << "	textureColor.a += v_color.a;" << std::endl;
			}
			break;
		case TEX0_FUNCTION_HIGHLIGHT2:
			shaderBuilder << "	textureColor.rgb = clamp(textureColor.rgb * v_color.rgb * 2.0, 0.0, 1.0) + v_color.aaa;" << std::endl;
			if(!caps.texHasAlpha)
			{
				shaderBuilder << "	textureColor.a = v_color.a;" << std::endl;
			}
			break;
		default:
			assert(0);
			break;
		}
	}
	else
	{
		shaderBuilder << "	textureColor = v_color;" << std::endl;
	}

	if(caps.hasAlphaTest)
	{
		shaderBuilder << GenerateAlphaTestSection(static_cast<ALPHA_TEST_METHOD>(caps.alphaTestMethod));
		//Check for early rejection
		if(caps.alphaFailResult == ALPHA_TEST_FAIL_KEEP)
		{
			shaderBuilder << "	if(alphaTestFail) discard;" << std::endl;
		}
	}

	if(caps.hasFog)
	{
		shaderBuilder << "	fragColor.xyz = mix(textureColor.rgb, g_fogColor, v_fog);" << std::endl;
	}
	else
	{
		shaderBuilder << "	fragColor.xyz = textureColor.xyz;" << std::endl;
	}

	//For proper alpha blending, alpha has to be multiplied by 2 (0x80 -> 1.0)
#ifdef GLES_COMPATIBILITY
	//This has the side effect of not writing a proper value in the framebuffer (should write alpha "as is")
	shaderBuilder << "	fragColor.a = clamp(textureColor.a * 2.0, 0.0, 1.0);" << std::endl;
#else
	shaderBuilder << "	fragColor.a = textureColor.a;" << std::endl;
#endif

	switch(orderingMode)
	{
	case FRAGMENT_SHADER_ORDERING_ARB:
		shaderBuilder << "	beginInvocationInterlockARB();" << std::endl;
		break;
	case FRAGMENT_SHADER_ORDERING_INTEL:
		shaderBuilder << "	beginFragmentShaderOrderingINTEL();" << std::endl;
		break;
	}

	shaderBuilder << "	ivec2 coords = ivec2(gl_FragCoord.xy);" << std::endl;

	//Depth test
	switch(caps.depthTestMethod)
	{
	case DEPTH_TEST_ALWAYS:
		break;
	case DEPTH_TEST_NEVER:
		shaderBuilder << "	depthTestFail = true;" << std::endl;
		break;
	case DEPTH_TEST_GEQUAL:
		shaderBuilder << "	uint depthValue = imageLoad(g_depthbuffer, coords).r;" << std::endl;
		shaderBuilder << "	depthTestFail = (depth < depthValue);" << std::endl;
		break;
	case DEPTH_TEST_GREATER:
		shaderBuilder << "	uint depthValue = imageLoad(g_depthbuffer, coords).r;" << std::endl;
		shaderBuilder << "	depthTestFail = (depth <= depthValue);" << std::endl;
		break;
	}

	//Update depth buffer
	if(caps.depthWriteEnabled)
	{
		const char* writeCondition = "	if(!depthTestFail)";
		if(caps.hasAlphaTest && (caps.alphaFailResult == ALPHA_TEST_FAIL_FBONLY))
		{
			writeCondition = "	if(!depthTestFail && !alphaTestFail)";
		}
		shaderBuilder << writeCondition << std::endl;
		shaderBuilder << "	{" << std::endl;
		shaderBuilder << "		imageStore(g_depthbuffer, coords, uvec4(depth & g_depthMask));" << std::endl;
		shaderBuilder << "	}" << std::endl;
	}

	shaderBuilder << "	if(!depthTestFail)" << std::endl;
	shaderBuilder << "	{" << std::endl;

	shaderBuilder << "		vec4 dstColor = imageLoad(g_framebuffer, coords);" << std::endl;

	if(caps.hasAlphaBlend)
	{
		shaderBuilder << "		vec3 colorA = " << GenerateAlphaBlendABDValue(static_cast<ALPHABLEND_ABD>(caps.blendFactorA)) << ";" << std::endl;
		shaderBuilder << "		vec3 colorB = " << GenerateAlphaBlendABDValue(static_cast<ALPHABLEND_ABD>(caps.blendFactorB)) << ";" << std::endl;
		shaderBuilder << "		vec3 colorD = " << GenerateAlphaBlendABDValue(static_cast<ALPHABLEND_ABD>(caps.blendFactorD)) << ";" << std::endl;
		shaderBuilder << "		float alphaC = " << GenerateAlphaBlendCValue(static_cast<ALPHABLEND_C>(caps.blendFactorC)) << ";" << std::endl;
		shaderBuilder << "		fragColor.xyz = ((colorA - colorB) * alphaC * 2) + colorD;" << std::endl;
	}

	shaderBuilder << "		if((g_colorMask & 0xFF000000) == 0) fragColor.a = dstColor.a;" << std::endl;
	shaderBuilder << "		if((g_colorMask & 0x00FF0000) == 0) fragColor.b = dstColor.b;" << std::endl;
	shaderBuilder << "		if((g_colorMask & 0x0000FF00) == 0) fragColor.g = dstColor.g;" << std::endl;
	shaderBuilder << "		if((g_colorMask & 0x000000FF) == 0) fragColor.r = dstColor.r;" << std::endl;

	shaderBuilder << "		imageStore(g_framebuffer, coords, fragColor);" << std::endl;
	shaderBuilder << "	}" << std::endl;

	switch(orderingMode)
	{
	case FRAGMENT_SHADER_ORDERING_ARB:
		shaderBuilder << "	endInvocationInterlockARB();" << std::endl;
		break;
	}

	shaderBuilder << "	discard;" << std::endl;

	shaderBuilder << "}" << std::endl;

	auto shaderSource = shaderBuilder.str();

	Framework::OpenGl::CShader result(GL_FRAGMENT_SHADER);
	result.SetSource(shaderSource.c_str(), shaderSource.size());
	FRAMEWORK_MAYBE_UNUSED bool compilationResult = result.Compile();
	assert(compilationResult);

	CHECKGLERROR();

	return result;
}

std::string CGSH_OpenGL::GenerateTexCoordClampingSection(TEXTURE_CLAMP_MODE clampMode, const char* coordinate)
{
	std::stringstream shaderBuilder;

	switch(clampMode)
	{
	case TEXTURE_CLAMP_MODE_REGION_CLAMP:
		shaderBuilder << "	texCoord." << coordinate << " = min(g_clampMax." << coordinate << ", "
		              << "max(g_clampMin." << coordinate << ", texCoord." << coordinate << "));" << std::endl;
		break;
	case TEXTURE_CLAMP_MODE_REGION_REPEAT:
		shaderBuilder << "	texCoord." << coordinate << " = or(int(and(int(texCoord." << coordinate << "), "
		              << "int(g_clampMin." << coordinate << "))), int(g_clampMax." << coordinate << "));";
		break;
	case TEXTURE_CLAMP_MODE_REGION_REPEAT_SIMPLE:
		shaderBuilder << "	texCoord." << coordinate << " = mod(texCoord." << coordinate << ", "
		              << "g_clampMin." << coordinate << ") + g_clampMax." << coordinate << ";" << std::endl;
		break;
	}

	std::string shaderSource = shaderBuilder.str();
	return shaderSource;
}

std::string CGSH_OpenGL::GenerateAlphaBlendABDValue(ALPHABLEND_ABD factor)
{
	switch(factor)
	{
	case ALPHABLEND_ABD_CS:
		return "fragColor.xyz";
		break;
	case ALPHABLEND_ABD_CD:
		return "dstColor.xyz";
		break;
	case ALPHABLEND_ABD_ZERO:
		return "vec3(0, 0, 0)";
		break;
	case ALPHABLEND_ABD_INVALID:
		assert(false);
		return "vec3(0, 0, 0)";
		break;
	}
}

std::string CGSH_OpenGL::GenerateAlphaBlendCValue(ALPHABLEND_C factor)
{
	switch(factor)
	{
	case ALPHABLEND_C_AS:
		return "fragColor.a";
		break;
	case ALPHABLEND_C_AD:
		return "dstColor.a";
		break;
	case ALPHABLEND_C_FIX:
		return "float(g_alphaFix) / 255.0";
		break;
	case ALPHABLEND_C_INVALID:
		assert(false);
		return "0";
		break;
	}
}

std::string CGSH_OpenGL::GenerateAlphaTestSection(ALPHA_TEST_METHOD testMethod)
{
	std::stringstream shaderBuilder;

	const char* test = "	alphaTestFail = false;";

	//testMethod is the condition to pass the test
	switch(testMethod)
	{
	case ALPHA_TEST_NEVER:
		test = "	alphaTestFail = true;";
		break;
	case ALPHA_TEST_ALWAYS:
		test = "	alphaTestFail = false;";
		break;
	case ALPHA_TEST_LESS:
		test = "	alphaTestFail = (textureColorAlphaInt >= g_alphaRef);";
		break;
	case ALPHA_TEST_LEQUAL:
		test = "	alphaTestFail = (textureColorAlphaInt > g_alphaRef);";
		break;
	case ALPHA_TEST_EQUAL:
		test = "	alphaTestFail = (textureColorAlphaInt != g_alphaRef);";
		break;
	case ALPHA_TEST_GEQUAL:
		test = "	alphaTestFail = (textureColorAlphaInt < g_alphaRef);";
		break;
	case ALPHA_TEST_GREATER:
		test = "	alphaTestFail = (textureColorAlphaInt <= g_alphaRef);";
		break;
	case ALPHA_TEST_NOTEQUAL:
		test = "	alphaTestFail = (textureColorAlphaInt == g_alphaRef);";
		break;
	default:
		assert(false);
		break;
	}

	shaderBuilder << "	uint textureColorAlphaInt = uint(textureColor.a * 255.0);" << std::endl;
	shaderBuilder << test << std::endl;

	std::string shaderSource = shaderBuilder.str();
	return shaderSource;
}

Framework::OpenGl::ProgramPtr CGSH_OpenGL::GeneratePresentProgram()
{
	Framework::OpenGl::CShader vertexShader(GL_VERTEX_SHADER);
	Framework::OpenGl::CShader pixelShader(GL_FRAGMENT_SHADER);

	{
		std::stringstream shaderBuilder;
		shaderBuilder << GLSL_VERSION << std::endl;
		shaderBuilder << "in vec2 a_position;" << std::endl;
		shaderBuilder << "in vec2 a_texCoord;" << std::endl;
		shaderBuilder << "out vec2 v_texCoord;" << std::endl;
		shaderBuilder << "uniform vec2 g_texCoordScale;" << std::endl;
		shaderBuilder << "void main()" << std::endl;
		shaderBuilder << "{" << std::endl;
		shaderBuilder << "	v_texCoord = a_texCoord * g_texCoordScale;" << std::endl;
		shaderBuilder << "	gl_Position = vec4(a_position, 0, 1);" << std::endl;
		shaderBuilder << "}" << std::endl;

		vertexShader.SetSource(shaderBuilder.str().c_str());
		FRAMEWORK_MAYBE_UNUSED bool result = vertexShader.Compile();
		assert(result);
	}

	{
		std::stringstream shaderBuilder;
		shaderBuilder << GLSL_VERSION << std::endl;
		shaderBuilder << "precision mediump float;" << std::endl;
		shaderBuilder << "in vec2 v_texCoord;" << std::endl;
		shaderBuilder << "out vec4 fragColor;" << std::endl;
		shaderBuilder << "uniform sampler2D g_texture;" << std::endl;
		shaderBuilder << "void main()" << std::endl;
		shaderBuilder << "{" << std::endl;
		shaderBuilder << "	fragColor = texture(g_texture, v_texCoord);" << std::endl;
		shaderBuilder << "}" << std::endl;

		pixelShader.SetSource(shaderBuilder.str().c_str());
		FRAMEWORK_MAYBE_UNUSED bool result = pixelShader.Compile();
		assert(result);
	}

	auto program = std::make_shared<Framework::OpenGl::CProgram>();

	{
		program->AttachShader(vertexShader);
		program->AttachShader(pixelShader);

		glBindAttribLocation(*program, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::POSITION), "a_position");
		glBindAttribLocation(*program, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::TEXCOORD), "a_texCoord");

		FRAMEWORK_MAYBE_UNUSED bool result = program->Link();
		assert(result);
	}

	return program;
}

Framework::OpenGl::ProgramPtr CGSH_OpenGL::GenerateCopyToFbProgram()
{
	Framework::OpenGl::CShader vertexShader(GL_VERTEX_SHADER);
	Framework::OpenGl::CShader pixelShader(GL_FRAGMENT_SHADER);

	{
		std::stringstream shaderBuilder;
		shaderBuilder << GLSL_VERSION << std::endl;
		shaderBuilder << "in vec2 a_position;" << std::endl;
		shaderBuilder << "in vec2 a_texCoord;" << std::endl;
		shaderBuilder << "out vec2 v_texCoord;" << std::endl;
		shaderBuilder << "uniform vec2 g_srcPosition;" << std::endl;
		shaderBuilder << "uniform vec2 g_srcSize;" << std::endl;
		shaderBuilder << "void main()" << std::endl;
		shaderBuilder << "{" << std::endl;
		shaderBuilder << "	v_texCoord = (a_texCoord * g_srcSize) + g_srcPosition;" << std::endl;
		shaderBuilder << "	gl_Position = vec4(a_position, 0, 1);" << std::endl;
		shaderBuilder << "}" << std::endl;

		vertexShader.SetSource(shaderBuilder.str().c_str());
		FRAMEWORK_MAYBE_UNUSED bool result = vertexShader.Compile();
		assert(result);
	}

	{
		std::stringstream shaderBuilder;
		shaderBuilder << GLSL_VERSION << std::endl;
		shaderBuilder << "precision mediump float;" << std::endl;
		shaderBuilder << "in vec2 v_texCoord;" << std::endl;
		shaderBuilder << "out vec4 fragColor;" << std::endl;
		shaderBuilder << "uniform sampler2D g_texture;" << std::endl;
		shaderBuilder << "void main()" << std::endl;
		shaderBuilder << "{" << std::endl;
		shaderBuilder << "	fragColor = texture(g_texture, v_texCoord);" << std::endl;
		shaderBuilder << "}" << std::endl;

		pixelShader.SetSource(shaderBuilder.str().c_str());
		FRAMEWORK_MAYBE_UNUSED bool result = pixelShader.Compile();
		assert(result);
	}

	auto program = std::make_shared<Framework::OpenGl::CProgram>();

	{
		program->AttachShader(vertexShader);
		program->AttachShader(pixelShader);

		glBindAttribLocation(*program, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::POSITION), "a_position");
		glBindAttribLocation(*program, static_cast<GLuint>(PRIM_VERTEX_ATTRIB::TEXCOORD), "a_texCoord");

		FRAMEWORK_MAYBE_UNUSED bool result = program->Link();
		assert(result);
	}

	return program;
}
