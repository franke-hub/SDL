<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2026 Frank Eskesen.
//
//       This file is free content, distributed under creative commons CC0,
//       explicitly released into the Public Domain.
//       (See accompanying html file LICENSE.ZERO or the original contained
//       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
//
// SPDX-License-Identifier: CC0-1.0
//----------------------------------------------------------------------------
//
// Title-
//       ~.IDEAS/README.md
//
// Purpose-
//       Idea desriptions.
//
// Last change date-
//       2026/05/01
//
//------------------------------------------------------------------------ -->
<!-- --------------------------------------------------------------------- -->

Copyright (C) 2026 Frank Eskesen.

This file is free content, distributed under creative commons CC0,
explicitly released into the Public Domain.
(See accompanying html file LICENSE.ZERO or the original contained

Ideas are sorted in date order rather than reverse date order.
(Oldest first, newest last)

### [The most recent idea](#most-recent)

## These are just ideas on how to do things. They are not necessarily *good*
ideas.

These generally originate in the middle of the night.

<!-- --------------------------------------------------------------------- -->
### 2026/05/01 Image comparison

#### Expertise
Not much. I tell you this becuase I don't know if the idea really makes any
sense.

I know *about* some image manipulation functions but have only uses libraries
to implement them.

I can use the jpeg library or Image Magick (version > 6) to decode
imagages and X11 to display them.
- jpeg decoding was written in 2007 and hasn't been looked at since 2021.
  - The more recent changes use the distributed jpeg library rather than
downloading and compiling it from source.
- ImageMagick decoding was written in 2018 and was updated in 2020 to account
for ImageMagick inteface changes.
downloading and compiling it from source.

#### How to compare images

Restrictions:
- Only valid for image to image comparison
- Useful for:
  - Duplicate picture detection
  -
- Not useful for:
  - Facial recognition
  - Images that are already small

Methodology (Compare image A to image B)
- Convert images to grey scale, enhancing edge detection by considering color
differences.
- Think about adjusting brightness of each image
  - Use average brightness?
  - Use maximum brightness?
  - Use minimum brightness?
- Since RGB values for each pixel are equal, only use one of them
- Compress images to a small size, say 128 x 128
- Ignore some low order bits. (Determine how many by testing)
- Sum the pixel byte to byte differences
- The sum of diffences measures image similarity.
- Test how well close matches do against larger compression sizes and/or
more low order bits

Testing:
- Begin with small data set
- See how well it works as is
- Repeat the procedure using less compression
- See how well that works, and how minimal compares to maximal compression
- Increase test data set size it sucessful

Either adjust the mechanism or discard it

Questions:
- Do we want to adjust for rotated images?
  - Adjust image correcting horizontal/vertical orientation
    - Is this possible?
  - Partially rotate one of the images, comparing each partial rotation
    - Test simple rotations first: 90, 180, 270 degrees
    - Need to find reasonable rotation angles
      - Will proabably need to generate a test set.
    - Stop if match found

<!-- --------------------------------------------------------------------- -->
### <a id="most-recent">2026/05/11 C++ Python</a>

Implement a Python-like object that can be passed.

- Garbage collected (Need some sort of scope control)
- Match python features and error conditions
  - Testing required
