/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
                           (C) 2002 - 2008 by Scott Wheeler (id3v2header.cpp)
    email                : kde@carewolf.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <iostream>
#include <bitset>

#include <taglib/toolkit/tstring.h>
#include <taglib/toolkit/tdebug.h>

#include "apefooter.h"

using namespace TagLib;
using namespace APE;

class APE::Footer::FooterPrivate
{
public:
  FooterPrivate() :
    version(0),
    footerPresent(true),
    headerPresent(false),
    isHeader(false),
    itemCount(0),
    tagSize(0) {}

  unsigned int version;

  bool footerPresent;
  bool headerPresent;

  bool isHeader;

  unsigned int itemCount;
  unsigned int tagSize;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

unsigned int APE::Footer::size()
{
  return 32;
}

ByteVector APE::Footer::fileIdentifier()
{
  return ByteVector("APETAGEX");
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

APE::Footer::Footer() :
  d(new FooterPrivate())
{
}

APE::Footer::Footer(const ByteVector &data) :
  d(new FooterPrivate())
{
  parse(data);
}

APE::Footer::~Footer()
{
  delete d;
}

unsigned int APE::Footer::version() const
{
  return d->version;
}

bool APE::Footer::headerPresent() const
{
  return d->headerPresent;
}

bool APE::Footer::footerPresent() const
{
  return d->footerPresent;
}

bool APE::Footer::isHeader() const
{
  return d->isHeader;
}

void APE::Footer::setHeaderPresent(bool b) const
{
  d->headerPresent = b;
}

unsigned int APE::Footer::itemCount() const
{
  return d->itemCount;
}

void APE::Footer::setItemCount(unsigned int s)
{
  d->itemCount = s;
}

unsigned int APE::Footer::tagSize() const
{
  return d->tagSize;
}

unsigned int APE::Footer::completeTagSize() const
{
  if(d->headerPresent)
    return d->tagSize + size();
  else
    return d->tagSize;
}

void APE::Footer::setTagSize(unsigned int s)
{
  d->tagSize = s;
}

void APE::Footer::setData(const ByteVector &data)
{
  parse(data);
}

ByteVector APE::Footer::renderFooter() const
{
  return render(false);
}

ByteVector APE::Footer::renderHeader() const
{
  if(!d->headerPresent)
    return ByteVector();
  else
    return render(true);
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void APE::Footer::parse(const ByteVector &data)
{
  if(data.size() < size())
    return;

  // The first eight bytes, data[0..7], are the File Identifier, "APETAGEX".

  // Read the version number

  d->version = data.toUInt(8, false);

  // Read the tag size

  d->tagSize = data.toUInt(12, false);

  // Read the item count

  d->itemCount = data.toUInt(16, false);

  // Read the flags

  std::bitset<32> flags(TAGLIB_CONSTRUCT_BITSET(data.toUInt(20, false)));

  d->headerPresent = flags[31];
  d->footerPresent = !flags[30];
  d->isHeader = flags[29];

}

ByteVector APE::Footer::render(bool isHeader) const
{
  ByteVector v;

  // add the file identifier -- "APETAGEX"

  v.append(fileIdentifier());

  // add the version number -- we always render a 2.000 tag regardless of what
  // the tag originally was.

  v.append(ByteVector::fromUInt(2000, false));

  // add the tag size

  v.append(ByteVector::fromUInt(d->tagSize, false));

  // add the item count

  v.append(ByteVector::fromUInt(d->itemCount, false));

  // render and add the flags

  std::bitset<32> flags;

  flags[31] = d->headerPresent;
  flags[30] = false; // footer is always present
  flags[29] = isHeader;

  v.append(ByteVector::fromUInt(flags.to_ulong(), false));

  // add the reserved 64bit

  v.append(ByteVector::fromLongLong(0));

  return v;
}
