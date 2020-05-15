#include "Precompiled.hpp"

#include "ZilchShader.hpp"

#include "ZilchFragment.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShaders/ZilchShadersStandard.hpp"
#include "SimpleZilchShaderIRGenerator.hpp"
#include "GraphicsBufferTypes.hpp"

//-------------------------------------------------------------------ZilchSpirVBackend
class ZilchSpirVBackend : public Zero::ZilchShaderIRBackend
{
public:
  ZilchSpirVBackend()
  {

  }

  Zero::String GetExtension() override
  {
    return "spv";
  }
  bool RunTranslationPass(Zero::ShaderTranslationPassResult& inputData, Zero::ShaderTranslationPassResult& outputData) override
  {
    outputData.mByteStream = inputData.mByteStream;
    outputData.mReflectionData = inputData.mReflectionData;
    return true;
  }
  Zero::String GetErrorLog() override
  {
    return "";
  }
};

//-------------------------------------------------------------------ZilchShaderManager
ZilchShaderManager::ZilchShaderManager()
{
  
}

ZilchShaderManager::~ZilchShaderManager()
{
  Destroy();
}

void ZilchShaderManager::Initialize(ZilchShaderInitData& initData)
{
  mFragmentFileManager = initData.mFragmentFileManager;
  mMaterialManager = initData.mMaterialManager;

  Zero::ShaderSettingsLibrary::InitializeInstance();
  Zero::ShaderSettingsLibrary::GetInstance().GetLibrary();

  Zero::SpirVNameSettings nameSettings;
  Zero::SimpleZilchShaderIRGenerator::LoadNameSettings(nameSettings);
  mShaderIRGenerator = new Zero::SimpleZilchShaderIRGenerator(new Zero::ZilchSpirVFrontEnd(), CreateZilchShaderSettings(nameSettings));

  Zilch::EventConnect(mShaderIRGenerator, Zero::Events::TranslationError, &ZilchShaderManager::OnTranslationError, this);
  Zilch::EventConnect(mShaderIRGenerator, Zilch::Events::CompilationError, &ZilchShaderManager::OnCompilationError, this);
  Zilch::EventConnect(mShaderIRGenerator, Zero::Events::ValidationError, &ZilchShaderManager::OnValidationError, this);

  auto pipeline = new Zero::ShaderPipelineDescription();
  pipeline->mDebugPasses.PushBack(new Zero::ZilchSpirVDisassemblerBackend());
  pipeline->mDebugPasses.PushBack(new Zero::SpirVValidatorPass());
  pipeline->mToolPasses.PushBack(new Zero::SpirVOptimizerPass());
  pipeline->mBackend = new ZilchSpirVBackend();
  mShaderIRGenerator->SetPipeline(pipeline);

  mShaderIRGenerator->SetupDependencies(initData.mShaderCoreDir);
}

void ZilchShaderManager::AddUniformDescriptor(Zilch::BoundType* boundType)
{
  mUniformDescriptors.PushBack(boundType);
}

ZilchShader* ZilchShaderManager::Find(const String& name)
{
  return mZilchShaderMap.FindValue(name, nullptr);
}

Zero::ZilchShaderIRType* ZilchShaderManager::FindFragmentType(const String& fragmentTypeName)
{
  return mShaderIRGenerator->FindFragmentType(fragmentTypeName);
}

HashMap<String, ZilchShader*>::valuerange ZilchShaderManager::Values()
{
  return mZilchShaderMap.Values();
}

void ZilchShaderManager::Destroy()
{
  for(ZilchShader* zilchSahder : mZilchShaderMap.Values())
    delete zilchSahder;
  mZilchShaderMap.Clear();
  delete mShaderIRGenerator;
}

void ZilchShaderManager::BuildFragmentsLibrary()
{
  for(ZilchFragmentFile* fragmentFile : mFragmentFileManager->mResourceMap.Values())
  {
    mShaderIRGenerator->AddFragmentCode(fragmentFile->mFileContents, fragmentFile->mFilePath, nullptr);
  }
  mShaderIRGenerator->CompileAndTranslateFragments();
}

void ZilchShaderManager::BuildShadersLibrary()
{
  for(ZilchMaterial* zilchMaterial : mMaterialManager->mMaterialMap.Values())
  {
    ComposeZilchMaterialShader(zilchMaterial);
  }

  mShaderIRGenerator->CompileAndTranslateShaders();
  mShaderIRGenerator->CompilePipeline();

  for(ZilchMaterial* zilchMaterial : mMaterialManager->mMaterialMap.Values())
  {
    CreateZilchMaterialShader(zilchMaterial);
  }
}

Zero::ZilchShaderSpirVSettings* ZilchShaderManager::CreateZilchShaderSettings(Zero::SpirVNameSettings& nameSettings)
{
  using namespace Zero;
  ZilchShaderSpirVSettings* settings = new ZilchShaderSpirVSettings(nameSettings);

  for(Zilch::BoundType* zilchType : mUniformDescriptors)
  {
    BufferDescription* bufferDescription = zilchType->Has<BufferDescription>();
    UniformBufferDescription uniformBufferDescription;
    uniformBufferDescription.Set(bufferDescription->mBindingId, 0, UniformBufferDescription::mAllStagesMask, zilchType->Name);

    for(auto range = zilchType->GetFields(); !range.Empty(); range.PopFront())
    {
      Zilch::Field* field = range.Front();
      Zilch::BoundType* fieldBoundType = Zilch::BoundType::GetBoundType(field->PropertyType);
      uniformBufferDescription.AddField(fieldBoundType, field->Name);
    }

    settings->AddUniformBufferDescription(uniformBufferDescription);
  }

  Zilch::BoundType* realType = ZilchTypeId(Zilch::Real);
  Zilch::BoundType* real2Type = ZilchTypeId(Zilch::Real2);
  Zilch::BoundType* real3Type = ZilchTypeId(Zilch::Real3);
  Zilch::BoundType* real4Type = ZilchTypeId(Zilch::Real4);
  Zilch::BoundType* intType = ZilchTypeId(Zilch::Integer);
  Zilch::BoundType* int4Type = ZilchTypeId(Zilch::Integer4);
  Zilch::BoundType* real4x4Type = ZilchTypeId(Zilch::Real4x4);

  settings->AutoSetDefaultUniformBufferDescription();

  // Add some default vertex definitions (glsl attributes)
  settings->mVertexDefinitions.AddField(real3Type, "LocalPosition");
  settings->mVertexDefinitions.AddField(real3Type, "LocalNormal");
  settings->mVertexDefinitions.AddField(real4Type, "Color");
  settings->mVertexDefinitions.AddField(real2Type, "Uv");
  settings->mVertexDefinitions.AddField(real4Type, "Aux0");

  // Set zilch fragment names for spirv built-ins
  settings->SetHardwareBuiltInName(spv::BuiltInPosition, nameSettings.mApiPerspectivePositionName);

  settings->SetMaxSimultaneousRenderTargets(4);
  settings->SetRenderTargetName("Target0", 0);
  settings->SetRenderTargetName("Target1", 1);
  settings->SetRenderTargetName("Target2", 2);
  settings->SetRenderTargetName("Target3", 3);

  // Set custom callbacks in both the compositor and entry point code generation
  // for dealing with perspective position vs. api perspective position.
  settings->mCallbackSettings.SetCompositeCallback(&ZilchShaderIRCompositor::ApiPerspectivePositionCallback, nullptr);
  settings->mCallbackSettings.SetAppendCallback(&Zero::EntryPointGeneration::PerspectiveTransformAppendVertexCallback, nullptr);

  settings->Finalize();

  return settings;
}

void ZilchShaderManager::ComposeZilchMaterialShader(ZilchMaterial* zilchMaterial)
{
  Zero::ZilchShaderIRCompositor::ShaderDefinition shaderDef;
  shaderDef.mShaderName = zilchMaterial->mMaterialName;
  // Colllect the fragment types described in the material
  for(MaterialFragment& materialFrag : zilchMaterial->mFragments)
  {
    Zero::ZilchShaderIRType* zilchFragment = mShaderIRGenerator->FindFragmentType(materialFrag.mFragmentName);
    if(zilchFragment == nullptr)
    {
      ErrorIf(true, "Failed to find fragment '%s'", materialFrag.mFragmentName.c_str());
      continue;
    }

    shaderDef.mFragments.PushBack(zilchFragment);
  }

  // Compose the shader
  Zero::ShaderCapabilities capabilities;
  mShaderIRGenerator->ComposeShader(shaderDef, capabilities);

  // Add each resultant zilch shader to the shader project
  for(size_t i = 0; i < Zero::FragmentType::Size; ++i)
  {
    Zero::ZilchShaderIRCompositor::ShaderStageDescription& stageDesc = shaderDef.mResults[i];
    if(!stageDesc.mShaderCode.Empty())
      mShaderIRGenerator->AddShaderCode(stageDesc.mShaderCode, stageDesc.mClassName, nullptr);
  }
}

void ZilchShaderManager::CreateZilchMaterialShader(ZilchMaterial* zilchMaterial)
{
  Zero::CreateDirectory("ShaderDebug");

  Zero::ZilchShaderIRCompositor::ShaderDefinition* shaderDef = mShaderIRGenerator->mShaderDefinitionMap.FindPointer(zilchMaterial->mMaterialName);
  ErrorIf(shaderDef == nullptr, "Failed to find a shader def for a created material");

  ZilchShader* zilchShader = new ZilchShader();
  zilchShader->mName = zilchMaterial->mMaterialName;
  zilchShader->mMaterial = zilchMaterial;
  for(size_t i = 0; i < Zero::FragmentType::Size; ++i)
  {
    Zero::ZilchShaderIRCompositor::ShaderStageDescription& stageDesc = shaderDef->mResults[i];
    if(stageDesc.mShaderCode.Empty())
      continue;

    Zero::ZilchShaderIRType* shaderType = mShaderIRGenerator->FindShaderType(stageDesc.mClassName);
    
    // Write the spirv disassembly to an output file for debugging
    Zero::SimpleZilchShaderIRGenerator::ShaderTranslationResult* translationResult = mShaderIRGenerator->mShaderResults.FindPointer(shaderType->mMeta->mZilchName);
    String fileName = Zero::FilePath::CombineWithExtension("ShaderDebug", shaderType->mMeta->mZilchName, ".spvtxt");
    Zero::WriteToFile(fileName, translationResult->mDebugResults[0]->mByteStream.ToString());

    // Save the byte code out
    Zero::ShaderTranslationPassResult* passResult = mShaderIRGenerator->FindTranslationResult(shaderType);
    passResult->mByteStream.SaveTo(zilchShader->mShaderByteCode[i]);
    
    // Save reflection info
    Zero::SimplifiedShaderReflectionData* simplifiedReflection = mShaderIRGenerator->FindSimplifiedReflectionResult(shaderType);
    zilchShader->mResources[i].mReflection = simplifiedReflection;
    zilchShader->mResources[i].mEntryPointName = shaderType->mEntryPoint->mEntryPointFn->mDebugResultName;
  }

  ExtractMaterialDescriptors(zilchShader);
 
  mZilchShaderMap[zilchShader->mName] = zilchShader;
}

void ZilchShaderManager::ExtractMaterialDescriptors(ZilchShader* zilchShader)
{
  HashMap<String, size_t> descriptorNameToId;
  Array<ZilchMaterialBindingDescriptor>& materialBindings = zilchShader->mBindingDescriptors;
  auto zilchGraphicsLibrary = Zilch::ZilchGraphicsLibrary::GetLibrary();

  HashMap<String, String> sampledImageValues;
  ZilchMaterial* zilchMaterial = zilchShader->mMaterial;
  for(MaterialFragment& fragment : zilchMaterial->mFragments)
  {
    Zero::ZilchShaderIRType* fragmentShaderType = mShaderIRGenerator->FindFragmentType(fragment.mFragmentName);
    ZilchShaderResources& shaderResources = zilchShader->mResources[fragmentShaderType->mMeta->mFragmentType];
    for(MaterialProperty& prop : fragment.mProperties)
    {
      if(prop.mType == ShaderPrimitiveType::SampledImage)
      {
        Array<Zero::ShaderResourceReflectionData*> results;
        shaderResources.mReflection->FindSampledImageReflectionData(fragmentShaderType, prop.mPropertyName, results);
        for(size_t i = 0; i < results.Size(); ++i)
          sampledImageValues[results[i]->mInstanceName] = String((const char*)prop.mData.Data());
      }
    }
  }

  auto extractFn = [this, &descriptorNameToId, &materialBindings, &zilchGraphicsLibrary, &sampledImageValues](Zero::ShaderStageResource& stageResource, ShaderStage::Enum shaderStage, MaterialDescriptorType backupDescriptorType)
  {
    String resourceName = stageResource.mReflectionData.mInstanceName;
    if(!descriptorNameToId.ContainsKey(resourceName))
    {
      descriptorNameToId.Insert(resourceName, materialBindings.Size());
      materialBindings.PushBack();
    }
    size_t index = descriptorNameToId[resourceName];
    ZilchMaterialBindingDescriptor& descriptor = materialBindings[index];
    descriptor.mName = stageResource.mReflectionData.mInstanceName;
    descriptor.mBindingId = stageResource.mReflectionData.mBinding;
    descriptor.mStageFlags = (descriptor.mStageFlags | ShaderStageEnumToFlags(shaderStage));
    descriptor.mSizeInBytes = stageResource.mReflectionData.mSizeInBytes;
    Zilch::BoundType* boundDescriptorType = zilchGraphicsLibrary->BoundTypes.FindValue(resourceName, nullptr);
    if(boundDescriptorType != nullptr)
    {
      BufferDescription* bufferDescription = boundDescriptorType->Has<BufferDescription>();
      descriptor.mDescriptorType = bufferDescription->mDescriptorType;
      descriptor.mBufferBindingType = bufferDescription->mBufferBindingType;
      descriptor.mOffsetInBytes = bufferDescription->mOffsetInBytes;
    }
    else
    {
      descriptor.mDescriptorType = backupDescriptorType;
      descriptor.mBufferBindingType = ShaderMaterialBindingId::Material;
    }

    if(descriptor.mDescriptorType == MaterialDescriptorType::SampledImage)
      descriptor.mSampledImageName = sampledImageValues[descriptor.mName];
  };

  
  for(size_t i = 0; i < ShaderStage::Count; ++i)
  {
    Zero::SimplifiedShaderReflectionData* simplifiedReflection = zilchShader->mResources[i].mReflection;
    Zero::ShaderStageInterfaceReflection& stageReflection = simplifiedReflection->mReflection;

    for(auto resource : stageReflection.mUniforms)
      extractFn(resource, (ShaderStage::Enum)i, MaterialDescriptorType::Uniform);
    for(auto resource : stageReflection.mSampledImages)
      extractFn(resource, (ShaderStage::Enum)i, MaterialDescriptorType::SampledImage);
  }
}

void ZilchShaderManager::OnTranslationError(Zero::TranslationErrorEvent* errorEvent, void* self)
{
  String errorMsg = BuildString(errorEvent->mShortMessage, ":\n", errorEvent->mFullMessage, "\n");
  errorMsg = errorEvent->mLocation.GetFormattedStringWithMessage(Zilch::MessageFormat::MsvcCpp, errorMsg);
  OnError(errorEvent->mLocation.Origin, errorMsg, self);
}

void ZilchShaderManager::OnCompilationError(Zilch::ErrorEvent* e, void* self)
{
  String errorMsg = BuildString(e->GetFormattedMessage(Zilch::MessageFormat::MsvcCpp), "\n");
  OnError(e->Location.Origin, errorMsg, self);
}

void ZilchShaderManager::OnValidationError(Zero::ValidationErrorEvent* e, void* self)
{
  String errorMsg = BuildString(e->GetFormattedMessage(Zilch::MessageFormat::MsvcCpp), "\n");
  OnError(e->mLocation.Origin, errorMsg, self);
}

void ZilchShaderManager::OnError(const String& codeLocation, const String& errorMsg, void* self)
{
  __debugbreak();
}
