#include <d3d11.h>
#include <cassert> // 런타임 도중에 에러를 검출하여 프로그램 폭파시키는 라이브러리
//#pragma comment(lib, "d3d11.lib") // USB느낌 너무 무거워서 .lib로 놓음 ( 누구나 접근하면 안되기도 하고) -> 속성 -> 링커 -> 모든옵션 -> 추가종속성 -> d3d11.lib; 하거나 이거
// 헤더는 솔루션에 외부 종속성에 이미 선언되있고, cpp느낌
#include "FreeImage.h"
#if not defined _DEBUG
#define MUST(Expression) (      (         (Expression)))
#else
#define MUST(Expression) (assert(SUCCEEDED(Expression)))
#endif

namespace Pipeline
{
    // Rendering Pipeline
    // 화면에 그래픽이 그려지는 단계를 의미합니다. (총 11가지)
    //  
    // IA -> VS -> RS -> PS -> OM
    //
    namespace
    {
        ID3D11Device* Device;
        ID3D11DeviceContext* DeviceContext;
        IDXGISwapChain* SwapChain;
        ID3D11RenderTargetView* RenderTargetView;  // 만들어놓은 윈도우 창 위에 띄워주는 실제 게임창?인듯

        namespace Buffer
        {
            ID3D11Buffer* Vertex; // GPU에서 사용할 데이터
            ID3D11Buffer* Constant[3];

            template<typename Data>
            void Update(ID3D11Buffer* buffer, Data const& data)
            {
                D3D11_MAPPED_SUBRESOURCE subResource = D3D11_MAPPED_SUBRESOURCE();
                MUST(DeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
                memcpy_s(subResource.pData, subResource.RowPitch, data, sizeof(data)); // 배열 전체만큼 초기화
                DeviceContext->Unmap(Buffer::Vertex, 0);
            }
        }
    }

    LRESULT CALLBACK Procedure(HWND const hWindow, UINT const uMessage, WPARAM const wParameter, LPARAM const lParameter)
    {
        switch (uMessage)
        {
        case WM_CREATE: // 이미지 여기에 넣어줌
        {
            {
                // 버퍼를 몇개를 스왑체인할꺼나 , 그 관리할 내용은 무엇이냐
                DXGI_SWAP_CHAIN_DESC descriptor = DXGI_SWAP_CHAIN_DESC();
                descriptor.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// ★★★ 어떤 데이터를 관리하고, 어떻게 읽어야되는지 결정? UNSIGNED NORMALIZE = 1로정규화한다.
                descriptor.SampleDesc.Count = 1; // 어떻게 안티앨리어싱을 결정할거냐 슈퍼샘플링/ -> 멀티샘플링 몇개로 이용할거냐
                descriptor.OutputWindow = hWindow; // 어디에 출력할거냐
                descriptor.Windowed = true;  // 창모드 사용할거냐
                descriptor.BufferCount = 1;  // 이중버퍼링 = 1 (기본적으로 버퍼가 포함되어있기 때문에)
                descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 버퍼의 사용 용도가 무엇이냐 , 완성한 렌더 타겟을 출력하는 용도

                //HRESULT hr = D3D11CreateDeviceAndSwapChain(); // RESULT에 대한 HANDLE 값
                MUST(D3D11CreateDeviceAndSwapChain  // 성공하면 넘겨주고 실패하면 런타임도중 폭파 , releas로 하면 매크로때문에 0으로 넣어짐
                (
                    nullptr, // adapter 그래픽카드 연결해주는곳 2D는 안쓰고 3D에서 사용 , nullptr == 내장그래픽으로 사용
                    D3D_DRIVER_TYPE_HARDWARE, // 디바이스 ,디바이스 컨텍스트가 누구로부터 만들어주나?
                    nullptr,
                    0,
                    nullptr,
                    0,
                    D3D11_SDK_VERSION, // 보내줘야 값을 검사해서 검사용도로 써줘야댐
                    &descriptor,
                    &SwapChain,
                    &Device,
                    nullptr,
                    &DeviceContext
                ));
            }
            {
                float const Coordinates[4][2]
                {
                    { -0.5f, +0.5f },
                    { +0.5f, +0.5f },
                    { -0.5f, -0.5f },
                    { +0.5f, -0.5f },
                };

                D3D11_BUFFER_DESC descriptor
                {
                    sizeof(Coordinates), // 공간설정
                    D3D11_USAGE_IMMUTABLE, // 활용 용도 결정 CPU에 대한 접근 자체를 막아줌 , GPU에서만 읽고 쓸 수 있는 데이터
                    D3D11_BIND_VERTEX_BUFFER // 해당 공간에 대한 식별자를 지정해줌
                };

                D3D11_SUBRESOURCE_DATA subResource{ Coordinates };

                ID3D11Buffer* buffer = nullptr;

                MUST(Device->CreateBuffer(&descriptor, &subResource, &buffer));

                const UINT Stride = sizeof(*Coordinates);  // stride 큰걸음 GPU에 좌표값을 비트로 치환해서 보내주긴 했는데, 얼만큼 끊어 읽어야 되는지 모름 , 그래서 어디서 끊어야 되는지 알려줌
                const UINT Offset = 0; // 어디서부터 읽어야되나 -> 처음부터라서 0 (처음시작위치 알려주는중)

                DeviceContext->IASetVertexBuffers(0, 1, &buffer, &Stride, &Offset);

                buffer->Release();
            }
            {
                D3D11_BUFFER_DESC descriptor
                {
                    sizeof(float[4][2]),
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_VERTEX_BUFFER,
                    D3D11_CPU_ACCESS_WRITE
                };

                MUST(Device->CreateBuffer(&descriptor, nullptr, &Buffer::Vertex));

                const UINT Stride = sizeof(float[2]);
                const UINT Offset = 0;

                DeviceContext->IASetVertexBuffers(1, 1, &Buffer::Vertex, &Stride, &Offset);
            }
            {
                DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            }
            {
                D3D11_BUFFER_DESC descriptor
                {
                    sizeof(float[4][4]),
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_CONSTANT_BUFFER,
                    D3D11_CPU_ACCESS_WRITE
                };

                for (UINT8 i = 0; i < 3; ++i)
                {
                    MUST(Device->CreateBuffer(&descriptor, nullptr, &Buffer::Constant[i]));
                    DeviceContext->VSSetConstantBuffers(i, 1, &Buffer::Constant[i]);
                }

                // Transform
                // View
                // Projection
            }
            {
                // 여기서부터 다시공부
#include "Shader/Bytecode/Vertex.h"
                {
                    D3D11_INPUT_ELEMENT_DESC Descriptor[2]
                    {
                        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0 },
                        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1 }
                    };

                    ID3D11InputLayout* inputLayout = nullptr;

                    MUST(Device->CreateInputLayout(Descriptor, 2, Bytecode, sizeof(Bytecode), &inputLayout));
                    DeviceContext->IASetInputLayout(inputLayout);

                    inputLayout->Release();
                }
                {
                    ID3D11VertexShader* vertexShader = nullptr;

                    MUST(Device->CreateVertexShader(Bytecode, sizeof(Bytecode), nullptr, &vertexShader));
                    DeviceContext->VSSetShader(vertexShader, nullptr, 0);

                    vertexShader->Release();
                }
            }
            {
#include "Shader/Bytecode/Pixel.h"
                ID3D11PixelShader* pixelShader = nullptr;

                MUST(Device->CreatePixelShader(Bytecode, sizeof(Bytecode), nullptr, &pixelShader));
                DeviceContext->PSSetShader(pixelShader, nullptr, 0);

                pixelShader->Release();
            }
            {
                FreeImage_Initialise();
                {
                    FIBITMAP* bitmap = FreeImage_Load(FREE_IMAGE_FORMAT::FIF_PNG, "penitent_idle.png");
                    {
                        FreeImage_FlipVertical(bitmap);

                        if (FreeImage_GetBPP(bitmap) != 32)
                        {
                            FIBITMAP* const previous = bitmap;
                            bitmap = FreeImage_ConvertTo32Bits(bitmap);
                            FreeImage_Unload(previous);
                        }

                        D3D11_TEXTURE2D_DESC descriptor = D3D11_TEXTURE2D_DESC();
                        descriptor.Width = FreeImage_GetWidth(bitmap);
                        descriptor.Height = FreeImage_GetHeight(bitmap);
                        descriptor.MipLevels = 1;
                        descriptor.ArraySize = 1;
                        descriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                        descriptor.SampleDesc.Count = 1;
                        descriptor.Usage = D3D11_USAGE_IMMUTABLE;
                        descriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                        D3D11_SUBRESOURCE_DATA subResource = D3D11_SUBRESOURCE_DATA();
                        subResource.pSysMem = FreeImage_GetBits(bitmap);
                        subResource.SysMemPitch = FreeImage_GetPitch(bitmap);

                        ID3D11Texture2D* texture2D = nullptr;

                        MUST(Device->CreateTexture2D(&descriptor, &subResource, &texture2D));
                        {
                            ID3D11ShaderResourceView* shaderResourceView = nullptr;
                            MUST(Device->CreateShaderResourceView(texture2D, nullptr, &shaderResourceView));
                            {
                                DeviceContext->PSSetShaderResources(0, 1, &shaderResourceView);
                            }
                            shaderResourceView->Release();
                        }
                        texture2D->Release();
                    }
                    FreeImage_Unload(bitmap);
                }
                FreeImage_DeInitialise();
            }
            return 0;
        }
        // 여기까지 다시공부


        case WM_SIZE:  // 창의 크기가 변경될때 ,창 최대화할때 캐릭터 크기변경 이나 캐릭터
        {
            {
                D3D11_VIEWPORT Viewport = D3D11_VIEWPORT();
                Viewport.Width = LOWORD(lParameter);
                Viewport.Height = HIWORD(lParameter);

                DeviceContext->RSSetViewports(1, &Viewport);
            }
            {
                ID3D11Texture2D* texture = nullptr; // 텍스쳐에 있는 아이디 식별번호 외우기 힘드니 따로 만들어준듯
                MUST(SwapChain->GetBuffer(0, IID_PPV_ARGS(&texture))); // 이중버퍼중 백버퍼 먼저쓰니 0, 텍스쳐에 있는 아이디 식별번호를 파악하여 백버퍼에서 일하게
                {
                    Device->CreateRenderTargetView(texture, nullptr, &RenderTargetView);
                }
                texture->Release();
                DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr); // OM에서 출력하므로 OM사용 , 렌더타겟 1개사용하므로 1 , 3D때배움 깊이자원
            }
            return 0;
        }
        case WM_APP:
        {
            {
                MUST(SwapChain->Present(0, 0)); // 백버퍼가 그리고있는 완성된 걸 보내주는 구문 , first == 딜레이주는거
                static const float Color[4] = { 0.0f, 0.0, 0.0f, 1.0f };
                DeviceContext->ClearRenderTargetView(RenderTargetView, Color);
            }

            // 영상 애니메이션 부분
            {
                static struct
                {
                    float const Width = 71;
                    float const Height = 76;
                }frame;

                static unsigned       count = 0;
                static const unsigned motion = 13;
                static const unsigned fpm = 700;

                float const Coordinates[4][2]
                {
                    { frame.Width * (count / fpm + 0), frame.Height * 0 }, // 좌상단
                    { frame.Width * (count / fpm + 1), frame.Height * 0 }, // 우상단
                    { frame.Width * (count / fpm + 0), frame.Height * 1 }, // 좌하단
                    { frame.Width * (count / fpm + 1), frame.Height * 1 }, // 우하단
                };

                Buffer::Update(Buffer::Vertex, Coordinates);


                count += 1;

                if (fpm * motion - 1 < count) count = 0;


            }

            {
                // 1 0 0 0
                // 0 1 0 0 
                // 0 0 1 0 
                // 0 0 0 1 -> 단위행렬

                float World[4][4] // 월드좌표
                {
                   1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 1
                };

                float View[4][4]
                {
                   1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 1
                };

                static float const x = 2.0f / 500.0f;
                static float const y = 2.0f / 500.0f;

                float Projection[4][4]
                {
                   x, 0, 0, 0,
                   0, y, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 1
                };

                Buffer::Update(Buffer::Constant[0], World);
                Buffer::Update(Buffer::Constant[0], View);
                Buffer::Update(Buffer::Constant[0], Projection);


                DeviceContext->Draw(4, 0);
            }
            return 0;
        }
        case WM_DESTROY:
        {
            Buffer::Vertex->Release();
            RenderTargetView->Release();
            Device->Release();
            DeviceContext->Release();
            SwapChain->Release();

            PostQuitMessage(0);
            return 0;
        }
        default:
        { return DefWindowProc(hWindow, uMessage, wParameter, lParameter); }
        }
    }
}