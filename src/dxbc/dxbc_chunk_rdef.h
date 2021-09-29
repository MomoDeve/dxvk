#pragma once

#include <cctype>

#include "dxbc_common.h"
#include "dxbc_decoder.h"
#include "dxbc_enums.h"
#include "dxbc_reader.h"
#include "dxbc_util.h"

namespace dxvk {

    struct DxbcRdefShaderVariable;

    struct DxbcRdefShaderVariableType {
        DxbcShaderVariableClass _class;
        DxbcShaderVariableType type;
        uint32_t rowCount;
        uint32_t columnCount;
        uint32_t elementCount;
        uint32_t memberCount;
        DxbcRdefShaderVariable* members = nullptr;
    };

    struct DxbcRdefShaderVariable {
        std::string name;
        DxbcRdefShaderVariableType type;
        DxbcShaderVariableFlag flags;
        uint32_t byteOffset;
        uint32_t byteSize;
        int32_t startTexture = -1;
        int32_t textureSize = 0;
        int32_t startSampler = -1;
        int32_t samplerSize = 0;
    };

    struct DxbcRdefConstantBuffer {
        std::string name;
        uint32_t byteSize;
        DxbcShaderCBufferFlag flags;
        DxbcCBufferType type;
        std::vector<DxbcRdefShaderVariable> variables;
    };

    struct DxbcRdefResourceBinding {
        std::string name;
        DxbcShaderInputType shaderInputType;
        DxbcResourceReturnType returnType;
        DxbcResourceDim viewDimension;
        uint32_t sampleCount;
        uint32_t bindPoint;
        uint32_t bindCount;
        DxbcShaderInputFlag shaderInputFlags;
    };

    class DxbcRdef : public RcObject {

    public:

        DxbcRdef(DxbcReader reader, DxbcTag tag);
        ~DxbcRdef();

        const DxbcRdefResourceBinding& getResourceBinding(uint32_t binding, DxbcBindingType type);

    private:
        DxbcRdefShaderVariable readVariable(const DxbcReader& reader, DxbcReader& variableReader);
        void destroyVariable(DxbcRdefShaderVariable& variable);
        DxbcProgramType toDxbcProgramType(uint16_t type);
        bool compareShaderInputTypeToBindingType(DxbcShaderInputType inputType, DxbcBindingType bindingType);

        std::string m_creator;
        uint32_t m_majorVersion = 0;
        uint32_t m_minorVersion = 0;
        DxbcShaderFlag m_flags = DxbcShaderFlag::None;
        DxbcProgramType m_programType = (DxbcProgramType)0;
        uint32_t m_interfaceSlotCount = 0;

        std::vector<DxbcRdefConstantBuffer> m_constantBuffers;
        std::vector<DxbcRdefResourceBinding> m_resourceBindings;

    };

}