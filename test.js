// Credit to https://stackoverflow.com/a/52171480/13458117. This is a 53-bit hash using floats in some tricky ways,
// which reduces the hash collision probability near certainty to rather unlikely, for our given space. The C++ code
// uses a 64-bit hash which is even more robust; that's necessary given its larger search space.
function hashPosition (position) {
  let h1 = 0xdeadbeef, h2 = 0x41c6ce57;

  for (let i = 0, value; i < position.length; i++) {
    value = position[i]
    h1 = Math.imul(h1 ^ value, 2654435761);
    h2 = Math.imul(h2 ^ value, 1597334677);
  }

  h1 = Math.imul(h1 ^ (h1>>>16), 2246822507) ^ Math.imul(h2 ^ (h2>>>13), 3266489909);
  h2 = Math.imul(h2 ^ (h2>>>16), 2246822507) ^ Math.imul(h1 ^ (h1>>>13), 3266489909);
  return 4294967296 * (2097151 & h2) + (h1>>>0);
}

// Consider the following position and given cut. We use the following indexing:

//   ###
//   ### ###

function getPositionWidth (position) {
  return position ? position[0] : 0
}

function getPositionHeight (position) {
  return position.length
}

function getPositionDims (position) {
  return { width: getPositionWidth(position), height: getPositionHeight(position) }
}

function prettyPrintPosition (position, {
  tileCharacter = '#',
  spaceCharacter = ' ',
  includeLabels = true,
  tileWidth = 3,
  tileHeight = 2,
  columnSpace = 1,
  rowSpace = 1
} = {}) {
  let { width, height } = getPositionDims(position)

  if (width === 0) return "Empty board."

  // We use an array of an array of an array of strings. The strings look like "####" and these are combined.
  const strings = []

  const tileString = [...new Array(tileHeight).keys()].map(() => tileCharacter.repeat(tileWidth))
  const emptyString = [...new Array(tileHeight).keys()].map(() => spaceCharacter.repeat(tileWidth))

  for (let row = 0; row < height; ++row) {
    const rowString = []
    let rowWidth = position[row]

    for (let col = 0; col < width; ++col) {
      rowString.push((col < rowWidth) ? tileString : emptyString)
    }

    strings.push(rowString)
  }

  // Convert the string array array array into a string
  

  // If labels
  return strings
}
