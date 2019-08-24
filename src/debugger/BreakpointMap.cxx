//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2019 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "BreakpointMap.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BreakpointMap::BreakpointMap(void)
  : myInitialized(false)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BreakpointMap::add(const Breakpoint& breakpoint, const uInt32 flags)
{
  myInitialized = true;
  myMap[breakpoint] = flags;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BreakpointMap::add(const uInt16 addr, const uInt8 bank, const uInt32 flags)
{
  add(Breakpoint(addr, bank), flags);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BreakpointMap::erase(const Breakpoint& breakpoint)
{
  myMap.erase(breakpoint);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BreakpointMap::erase(const uInt16 addr, const uInt8 bank)
{
  erase(Breakpoint(addr, bank));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 BreakpointMap::get(const Breakpoint& breakpoint) const
{
  auto find = myMap.find(breakpoint);
  if(find != myMap.end())
    return find->second;

  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 BreakpointMap::get(uInt16 addr, uInt8 bank) const
{
  return get(Breakpoint(addr, bank));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool BreakpointMap::check(const Breakpoint & breakpoint) const
{
  auto find = myMap.find(breakpoint);

  return (find != myMap.end());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool BreakpointMap::check(const uInt16 addr, const uInt8 bank) const
{
  return check(Breakpoint(addr, bank));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BreakpointMap::BreakpointList BreakpointMap::getBreakpoints() const
{
  BreakpointList map;

  for(auto item : myMap)
    map.push_back(item.first);

  return map;
}
