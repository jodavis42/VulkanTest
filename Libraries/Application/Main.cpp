#include "Precompiled.hpp"

#include "Application.hpp"

int main()
{
  ApplicationConfig config;
  Application app(&config);

  app.Run();

  return EXIT_SUCCESS;
}
