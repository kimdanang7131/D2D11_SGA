// 확장 -> 확장관리 -> hlsl -> hlsl tools for visual studio 적용 하면 다운로드
#include "Layout.hlsli"

namespace Shader
{
	Layout::Pixel Vertex(const Layout::Vertex Input)
	{
		Layout::Pixel Output =
		{
			Input.Position,
			Input.Texcoord
		};
		
		return Output;
	}

}