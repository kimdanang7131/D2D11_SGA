// Ȯ�� -> Ȯ����� -> hlsl -> hlsl tools for visual studio ���� �ϸ� �ٿ�ε�
#include "Layout.hlsli"

namespace Shader
{
	// matrix�������� float 4x4�� �о��ֱ� ���ؼ�
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