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

function getPositionSquareCount (position) {
  let sum = 0
  for (let i = 0; i < position.length; ++i {
    sum += position[i]
  }
  return sum
}

function restrictStringTo (str, length) {
  if (str.length === length) {
    return str
  } else if (str.length > length) {
    return str.substr(0, length)
  } else {
    return str + ' '.repeat(length - str.length)
  }
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
  let strings = []

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

  function generateColLabel(label) {
    label = '' + label

    let stringArr = emptyString.slice()
    let lastIndex = stringArr.length - 1

    let string = Array.from(stringArr[lastIndex])

    let startSubindex = Math.floor(string.length / 2 - label.length / 2)
    for (let i = startSubindex, j=0; i < startSubindex + label.length; ++j, ++i) {
      string[i] = label[j]
    }

    stringArr[lastIndex] = string.join('')

    return stringArr
  }

  function generateRowLabel(label) {
    label = '' + label

    let stringArr = emptyString.slice()
    let subrowIndex = Math.floor(height / 2)

    let subrowString = ' '.repeat(tileWidth - label.length) + label

    stringArr[subrowIndex] = subrowString

    return stringArr
  }


  if (includeLabels) {
    // Prepend an extra row to the string containing all of the column numbers

    const extraRow = [ emptyString ].concat([...new Array(width).keys()].map(i => generateColLabel(i + 1)))

    for (let i = 0; i < height; ++i) {
      const row = strings[i]

      row.unshift(generateRowLabel(i + 1))
    }

    strings.push(extraRow)
  }

  strings = strings.reverse()

  // Convert the string array array array into a string

  let out = []

  for (let row of strings) {
    let rowOut = []

    for (let i = 0; i < tileHeight; ++i) {
      const subrowOut = []

      for (let col of row) {
        subrowOut.push(restrictStringTo(col[i], tileWidth))
      }

      rowOut.push(subrowOut.join(' '.repeat(columnSpace)))
    }

    rowOut = rowOut.join('\n')

    out.push(rowOut)
  }

  out = out.join('\n'.repeat(rowSpace + 1))

  // If labels
  return out
}

// Map between <hash(position), { winning: true/false }
const PositionData = new Map()

function getDataOf(position) {
  return PositionData.get(hashPosition(position))
}

function setDataOf(position, data) {
  PositionData.set(hashPosition(position), data)
}

// credit to geeks for geeks
function* positionsWithNTiles(n)  {
  let partition = [n]

  // Length of the current partition - 1
  let len = 0

  while (true) {
    yield partition.slice()

    let rem_val = 0

    while (len >= 0 && partition[len] === 1)
    {
      rem_val += partition[len]
      len--
    }

    if (len < 0)
      return

    partition[len]--
    rem_val++
    while (rem_val > partition[len])
    {
      partition[len + 1] = partition[len]
      rem_val = rem_val - partition[len]
      len++
    }
    partition[len + 1] = rem_val
    len++
  }
}

function* positionsStartingFromPositionWithNTiles (position, n) {
  let positionSquareCount = getPositionSquareCount(position)

  if (n.length > positionSquareCount) {
    return
  }

  const currentPotentialPosition = []

  // The strategy is to recursively iterate through all output_position[0] starting from position[0] down to 0, keeping
  // track of how many squares have already been used up, and comparing at each point to position[n]. We need to
  // allocate a certain minimum of tiles each time or we will end up with excess squares. If we are finding the value
  // of output_position[i] we need to know

  function  (i) {

  }
}

// Could do via memoization, but... eh...
function* positionsStartingFrom (position) {
  // We start from the initial position and generate all
}
