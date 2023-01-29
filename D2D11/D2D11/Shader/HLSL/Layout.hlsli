namespace Layout
{
	// 1 , 1 , 0 , 0 , 1 , 0 , 0 , 1
	
	struct Vertex
	{
		// Sementic : HLSL ���� �������� �����̳� �ǹ̸� �ο��ϴ� ����Դϴ�.
		//            ��ǻ�ʹ� �ش� �ø�ƽ ������ ���� � �������� Ȯ���մϴ�.
		float4 Position : POSITION;
		float4 Texcoord : TEXCOORD;
	};
	
	struct Pixel
	{
		float4 Position : SV_POSITION;
		float4 Texcoord : TEXCOORD;
	};
	
	typedef float4 Color;
}