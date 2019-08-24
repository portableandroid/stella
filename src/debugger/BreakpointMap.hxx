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

#ifndef BREAKPOINT_HXX
#define BREAKPOINT_HXX

#include <unordered_map>

#include "bspf.hxx"

/**
  This class handles simple debugger breakpoints.

  @author  Thomas Jentzsch
*/
class BreakpointMap
{
private:
  static const uInt16 ADDRESS_MASK = 0x1fff; // either 0x1fff or 0xffff (not needed then)

public:
  // breakpoint flags
  static const uInt32 ONE_SHOT = 1 << 0; // used to 'trace' command

  struct Breakpoint
  {
    uInt16 addr;
    uInt8 bank;

    Breakpoint()
      : addr(0), bank(0) { }
    Breakpoint(const Breakpoint& bp)
      : addr(bp.addr), bank(bp.bank) { }
    explicit Breakpoint(uInt16 c_addr, uInt8 c_bank)
      : addr(c_addr), bank(c_bank) { }

    bool operator==(const Breakpoint& other) const
    {
      return ((addr & ADDRESS_MASK) == (other.addr & ADDRESS_MASK) && bank == other.bank);
    }
  };
  using BreakpointList = std::vector<Breakpoint>;

  BreakpointMap();
  virtual ~BreakpointMap() = default;

  bool isInitialized() const { return myInitialized; }

  /** Add new breakpoint */
  void add(const Breakpoint& breakpoint, const uInt32 flags = 0);
  void add(const uInt16 addr, const uInt8 bank, const uInt32 flags = 0);

  /** Erase breakpoint */
  void erase(const Breakpoint& breakpoint);
  void erase(const uInt16 addr, const uInt8 bank);

  /** Get info for breakpoint */
  uInt32 get(const Breakpoint& breakpoint) const;
  uInt32 get(uInt16 addr, uInt8 bank) const;

  /** Check if a breakpoint exists */
  bool check(const Breakpoint& breakpoint) const;
  bool check(const uInt16 addr, const uInt8 bank) const;

  BreakpointList getBreakpoints() const;

  /** clear all breakpoints */
  void clear() { myMap.clear(); }
  size_t size() { return myMap.size(); }

private:
  struct BreakpointHash {
    size_t operator()(const Breakpoint& bp) const {
      return std::hash<uInt64>()(
        uInt64(bp.addr & ADDRESS_MASK) * 13 + uInt64(bp.bank)
      );
    }
  };

  std::unordered_map<Breakpoint, uInt32, BreakpointHash> myMap;
  bool myInitialized;
};

#endif
