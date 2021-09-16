#include "dxbc_chunk_rdef.h"

#undef min
#undef max

namespace dxvk {

    DxbcRdef::DxbcRdef(DxbcReader reader, DxbcTag tag) {
        uint32_t constantBufferCount = reader.readu32();
        uint32_t constantBufferDescriptionOffset = reader.readu32();
        uint32_t resourceBindingCount = reader.readu32();
        uint32_t resourceBindingOffset = reader.readu32();
        m_minorVersion = (uint32_t)reader.readu8();
        m_majorVersion = (uint32_t)reader.readu8();
        m_programType = toDxbcProgramType(reader.readu16());
        m_flags = (DxbcShaderFlag)reader.readu32();
        m_creator = reader.clone(reader.readu32()).readString();

        // from https://github.com/tgjones/slimshader/blob/master/src/SlimShader/Chunks/Rdef/ResourceDefinitionChunk.cs#L50
        if (m_majorVersion >= 5) {
            auto tag = reader.readTag(); // should be RD11

            uint32_t unknown1 = reader.readu32(); // should be 60
            uint32_t unknown2 = reader.readu32(); // should be 24
            uint32_t unknown3 = reader.readu32(); // should be 32
            uint32_t unknown4 = reader.readu32(); // should be 40
            uint32_t unknown5 = reader.readu32(); // should be 36
            uint32_t unknown6 = reader.readu32(); // should be 12

            m_interfaceSlotCount = reader.readu32();
        }

        DxbcReader constantBufferReader = reader.clone(constantBufferDescriptionOffset);
        for (uint32_t i = 0; i < constantBufferCount; i++)
        {
            DxbcRdefConstantBuffer cbuffer;
            cbuffer.name = reader.clone(constantBufferReader.readu32()).readString();
            uint32_t variablesCount = constantBufferReader.readu32();
            uint32_t variablesOffset = constantBufferReader.readu32();
            cbuffer.byteSize = constantBufferReader.readu32();
            cbuffer.flags = (DxbcShaderCBufferFlag)constantBufferReader.readu32();
            cbuffer.type = (DxbcCBufferType)constantBufferReader.readu32();

            DxbcReader variableReader = reader.clone(variablesOffset);
            for (uint32_t j = 0; j < variablesCount; j++)
            {
                DxbcRdefShaderVariable variable = readVariable(reader, variableReader);
                cbuffer.variables.push_back(variable);
            }

            m_constantBuffers.push_back(cbuffer);
        }

        DxbcReader resourceBindingReader = reader.clone(resourceBindingOffset);
        for (uint32_t i = 0; i < resourceBindingCount; i++)
        {
            DxbcRdefResourceBinding binding;
            binding.name = reader.clone(resourceBindingReader.readu32()).readString();
            binding.shaderInputType = (DxbcShaderInputType)resourceBindingReader.readu32();
            binding.returnType = (DxbcResourceReturnType)resourceBindingReader.readu32();
            binding.viewDimension = (DxbcResourceDim)resourceBindingReader.readu32();
            binding.sampleCount = resourceBindingReader.readu32();
            binding.bindPoint = resourceBindingReader.readu32();
            binding.bindCount = resourceBindingReader.readu32();
            binding.shaderInputFlags = (DxbcShaderInputFlag)resourceBindingReader.readu32();

            m_resourceBindings.push_back(binding);
        }
    }

    DxbcRdefShaderVariable DxbcRdef::readVariable(const DxbcReader& reader, DxbcReader& variableReader) {
        DxbcRdefShaderVariable variable;
        variable.name = reader.clone(variableReader.readu32()).readString();
        variable.byteOffset = variableReader.readu32();
        variable.byteSize = variableReader.readu32();
        variable.flags = (DxbcShaderVariableFlag)variableReader.readu32();
        DxbcReader variableTypeReader = reader.clone(variableReader.readu32());
        uint32_t variableDefaultValueOffset = variableReader.readu32();

        variable.type._class = (DxbcShaderVariableClass)variableTypeReader.readu16();
        variable.type.type = (DxbcShaderVariableType)variableTypeReader.readu16();
        variable.type.rowCount = variableTypeReader.readu16();
        variable.type.columnCount = variableTypeReader.readu16();
        variable.type.elementCount = variableTypeReader.readu16();
        variable.type.memberCount = (uint32_t)variableTypeReader.readu16();

        variable.type.members = new DxbcRdefShaderVariable[variable.type.memberCount];
        DxbcReader variableMemberReader = reader.clone(variableTypeReader.readu16());
        for (uint32_t i = 0; i < variable.type.memberCount; i++)
        {
            variable.type.members[i] = readVariable(reader, variableMemberReader);
        }

        if (m_majorVersion >= 5)
        {
            variable.startTexture = variableReader.readi32();
            variable.textureSize = variableReader.readi32();
            variable.startSampler = variableReader.readi32();
            variable.samplerSize = variableReader.readi32();
        }

        return variable;
    }

    DxbcProgramType DxbcRdef::toDxbcProgramType(uint16_t type) {
        switch (type) {
        case 0xFFFF:
            return DxbcProgramType::PixelShader;
        case 0xFFFE:
            return DxbcProgramType::VertexShader;
        case 0x4853:
            return DxbcProgramType::HullShader;
        case 0x4753:
            return DxbcProgramType::GeometryShader;
        case 0x4453:
            return DxbcProgramType::DomainShader;
        case 0x4353:
            return DxbcProgramType::ComputeShader;
        default:
            return (DxbcProgramType)0;
        }
    }

    void DxbcRdef::destroyVariable(DxbcRdefShaderVariable& variable) {
        for (uint32_t i = 0; i < variable.type.memberCount; i++) {
            destroyVariable(variable.type.members[i]);
        }
        delete[] variable.type.members;
    }

    DxbcRdef::~DxbcRdef() {
        for (uint32_t i = 0; i < m_constantBuffers.size(); i++) {
            for (uint32_t j = 0; j < m_constantBuffers[i].variables.size(); j++) {
                destroyVariable(m_constantBuffers[i].variables[j]);
            }
        }
    }

}