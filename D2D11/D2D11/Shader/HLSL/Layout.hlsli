namespace Layout
{
	// 1 , 1 , 0 , 0 , 1 , 0 , 0 , 1
	
	struct Vertex
	{
		// Sementic : HLSL 에서 데이터의 역할이나 의미를 부여하는 기능입니다.
		//            컴퓨터는 해당 시멘틱 네임을 통해 어떤 역할인지 확인합니다.
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