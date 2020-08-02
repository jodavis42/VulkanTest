#include "Precompiled.hpp"

#include "Cookie.hpp"

//-------------------------------------------------------------------Cookie
size_t Cookie::GetCookie() const
{
  return mCookie;
}

size_t Cookie::Hash() const
{
  return Zero::HashPolicy<size_t>()(mCookie);
}

Cookie::Cookie()
{

}

Cookie::Cookie(size_t cookie)
  : mCookie(cookie)
{

}
