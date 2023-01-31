#include <d3d11.h>
#include <cassert> // ��Ÿ�� ���߿� ������ �����Ͽ� ���α׷� ���Ľ�Ű�� ���̺귯��
//#pragma comment(lib, "d3d11.lib") // USB���� �ʹ� ���ſ��� .lib�� ���� ( ������ �����ϸ� �ȵǱ⵵ �ϰ�) -> �Ӽ� -> ��Ŀ -> ���ɼ� -> �߰����Ӽ� -> d3d11.lib; �ϰų� �̰�
// ����� �ַ�ǿ� �ܺ� ���Ӽ��� �̹� ������ְ�, cpp����
#include "FreeImage.h"
#if not defined _DEBUG
#define MUST(Expression) (      (         (Expression)))
#else
#define MUST(Expression) (assert(SUCCEEDED(Expression)))
#endif

namespace Pipeline
{
    // Rendering Pipeline
    // ȭ�鿡 �׷����� �׷����� �ܰ踦 �ǹ��մϴ�. (�� 11����)
    //  
    // IA -> VS -> RS -> PS -> OM
    //
    namespace
    {
        ID3D11Device* Device;
        ID3D11DeviceContext* DeviceContext;
        IDXGISwapChain* SwapChain;
        ID3D11RenderTargetView* RenderTargetView;  // �������� ������ â ���� ����ִ� ���� ����â?�ε�

        namespace Buffer
        {
            ID3D11Buffer* Vertex; // GPU���� ����� ������
            ID3D11Buffer* Constant[3];

            template<typename Data>
            void Update(ID3D11Buffer* buffer, Data const& data)
            {
                D3D11_MAPPED_SUBRESOURCE subResource = D3D11_MAPPED_SUBRESOURCE();
                MUST(DeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
                memcpy_s(subResource.pData, subResource.RowPitch, data, sizeof(data)); // �迭 ��ü��ŭ �ʱ�ȭ
                DeviceContext->Unmap(Buffer::Vertex, 0);
            }
        }
    }

    LRESULT CALLBACK Procedure(HWND const hWindow, UINT const uMessage, WPARAM const wParameter, LPARAM const lParameter)
    {
        switch (uMessage)
        {
        case WM_CREATE: // �̹��� ���⿡ �־���
        {
            {
                // ���۸� ��� ����ü���Ҳ��� , �� ������ ������ �����̳�
                DXGI_SWAP_CHAIN_DESC descriptor = DXGI_SWAP_CHAIN_DESC();
                descriptor.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// �ڡڡ� � �����͸� �����ϰ�, ��� �о�ߵǴ��� ����? UNSIGNED NORMALIZE = 1������ȭ�Ѵ�.
                descriptor.SampleDesc.Count = 1; // ��� ��Ƽ�ٸ������ �����Ұų� ���ۻ��ø�/ -> ��Ƽ���ø� ��� �̿��Ұų�
                descriptor.OutputWindow = hWindow; // ��� ����Ұų�
                descriptor.Windowed = true;  // â��� ����Ұų�
                descriptor.BufferCount = 1;  // ���߹��۸� = 1 (�⺻������ ���۰� ���ԵǾ��ֱ� ������)
                descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ������ ��� �뵵�� �����̳� , �ϼ��� ���� Ÿ���� ����ϴ� �뵵

                //HRESULT hr = D3D11CreateDeviceAndSwapChain(); // RESULT�� ���� HANDLE ��
                MUST(D3D11CreateDeviceAndSwapChain  // �����ϸ� �Ѱ��ְ� �����ϸ� ��Ÿ�ӵ��� ���� , releas�� �ϸ� ��ũ�ζ����� 0���� �־���
                (
                    nullptr, // adapter �׷���ī�� �������ִ°� 2D�� �Ⱦ��� 3D���� ��� , nullptr == ����׷������� ���
                    D3D_DRIVER_TYPE_HARDWARE, // ����̽� ,����̽� ���ؽ�Ʈ�� �����κ��� ������ֳ�?
                    nullptr,
                    0,
                    nullptr,
                    0,
                    D3D11_SDK_VERSION, // ������� ���� �˻��ؼ� �˻�뵵�� ����ߴ�
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
                    sizeof(Coordinates), // ��������
                    D3D11_USAGE_IMMUTABLE, // Ȱ�� �뵵 ���� CPU�� ���� ���� ��ü�� ������ , GPU������ �а� �� �� �ִ� ������
                    D3D11_BIND_VERTEX_BUFFER // �ش� ������ ���� �ĺ��ڸ� ��������
                };

                D3D11_SUBRESOURCE_DATA subResource{ Coordinates };

                ID3D11Buffer* buffer = nullptr;

                MUST(Device->CreateBuffer(&descriptor, &subResource, &buffer));

                const UINT Stride = sizeof(*Coordinates);  // stride ū���� GPU�� ��ǥ���� ��Ʈ�� ġȯ�ؼ� �����ֱ� �ߴµ�, ��ŭ ���� �о�� �Ǵ��� �� , �׷��� ��� ����� �Ǵ��� �˷���
                const UINT Offset = 0; // ��𼭺��� �о�ߵǳ� -> ó�����Ͷ� 0 (ó��������ġ �˷��ִ���)

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
                // ���⼭���� �ٽð���
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
        // ������� �ٽð���


        case WM_SIZE:  // â�� ũ�Ⱑ ����ɶ� ,â �ִ�ȭ�Ҷ� ĳ���� ũ�⺯�� �̳� ĳ����
        {
            {
                D3D11_VIEWPORT Viewport = D3D11_VIEWPORT();
                Viewport.Width = LOWORD(lParameter);
                Viewport.Height = HIWORD(lParameter);

                DeviceContext->RSSetViewports(1, &Viewport);
            }
            {
                ID3D11Texture2D* texture = nullptr; // �ؽ��Ŀ� �ִ� ���̵� �ĺ���ȣ �ܿ�� ����� ���� ������ص�
                MUST(SwapChain->GetBuffer(0, IID_PPV_ARGS(&texture))); // ���߹����� ����� �������� 0, �ؽ��Ŀ� �ִ� ���̵� �ĺ���ȣ�� �ľ��Ͽ� ����ۿ��� ���ϰ�
                {
                    Device->CreateRenderTargetView(texture, nullptr, &RenderTargetView);
                }
                texture->Release();
                DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr); // OM���� ����ϹǷ� OM��� , ����Ÿ�� 1������ϹǷ� 1 , 3D����� �����ڿ�
            }
            return 0;
        }
        case WM_APP:
        {
            {
                MUST(SwapChain->Present(0, 0)); // ����۰� �׸����ִ� �ϼ��� �� �����ִ� ���� , first == �������ִ°�
                static const float Color[4] = { 0.0f, 0.0, 0.0f, 1.0f };
                DeviceContext->ClearRenderTargetView(RenderTargetView, Color);
            }

            // ���� �ִϸ��̼� �κ�
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
                    { frame.Width * (count / fpm + 0), frame.Height * 0 }, // �»��
                    { frame.Width * (count / fpm + 1), frame.Height * 0 }, // ����
                    { frame.Width * (count / fpm + 0), frame.Height * 1 }, // ���ϴ�
                    { frame.Width * (count / fpm + 1), frame.Height * 1 }, // ���ϴ�
                };

                Buffer::Update(Buffer::Vertex, Coordinates);


                count += 1;

                if (fpm * motion - 1 < count) count = 0;


            }

            {
                // 1 0 0 0
                // 0 1 0 0 
                // 0 0 1 0 
                // 0 0 0 1 -> �������

                float World[4][4] // ������ǥ
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