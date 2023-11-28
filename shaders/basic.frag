#version 330

in vec2 UV;

out vec4 out_Color;

uniform sampler2D u_Texture;
uniform uint      u_FrameNo;
uniform ivec2     u_FrameWidthHeight;

void main()
{
	// out_Color = texture(u_Texture, UV);
	out_Color = vec4(1.0, 0.0, 0.0, 1.0);
}
