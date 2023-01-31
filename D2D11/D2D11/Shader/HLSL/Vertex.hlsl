// 확장 -> 확장관리 -> hlsl -> hlsl tools for visual studio 적용 하면 다운로드
#include "Layout.hlsli"

namespace Shader
{
	// matrix쓰는이유 float 4x4로 읽어주기 위해서
    cbuffer Transform : register(B0)
    {
        matrix World;
    }
    cbuffer Transform : register(B1)
    {
        matrix View;
    }
    cbuffer Transform : register(B2)
    {
        matrix Projection;
    }
		
    Layout::Pixel Vertex(const Layout::Vertex Input)
    {
        Layout::Pixel Output =
        {
            Input.Position,
			Input.Texcoord
        };
		
        Output.Position = mul(Output.Position, World);
        Output.Position = mul(Output.Position, View);
        Output.Position = mul(Output.Position, Projection);
		
        return Output;
    }

}