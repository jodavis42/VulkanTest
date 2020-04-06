#pragma once

struct VulkanRuntimeData;

class VulkanRenderer
{
public:
  VulkanRenderer();
  ~VulkanRenderer();

  void Initialize();
  void Destroy();

  void Draw();
private:

  VulkanRuntimeData* mInternal;
};
