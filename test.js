// Credit to https://stackoverflow.com/a/52171480/13458117. This is a 53-bit hash using floats in some tricky ways,
// which reduces the hash collision probability near certainty to rather unlikely, for our given space. The C++ code
// uses a 64-bit hash which is even more robust; that's necessary given its larger search space.
function hashPosition (position) {
  let h1 = 0xdeadbeef, h2 = 0x41c6ce57;

  for (let i = 0, value; i < position.length; i++) {
    value = position[i]
    if (value === 0) break

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
  for (let i = 0; i < position.length; ++i) {
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

// Remember that row = 0, col = 0 is the poisoned square
function cutPosition (position, row, col) {
  let clone = position.slice()

  // This corresponds to setting all the values at and above index *row* to min(val, col)
  for (let i = row; i < position.length; ++i) {
    clone[i] = Math.min(clone[i], col)
  }

  return clone
}

function getAllCuts (position, ret) {
  for (let i = 0; i < position.length; ++i) {
    let colWidth = position[i]

    for (let j = 0; j < colWidth; ++j) {
      ret(i, j)
    }
  }
}

function getAllCuttedPositions (position, ret) {
  getAllCuts(position, (x, y) => {
    let cuttedPosition = cutPosition(position, x, y)

    ret(x, y, cuttedPosition)
  })
}

function getAllWinningCuts (position, ret) {
  getAllCuttedPositions(position, (x, y, pos) => {
    if (!getDataOf(pos).isWinning) {
      ret(x, y, pos)
    }
  })
}

function getAllLosingCuts (position, ret) {
  getAllCuttedPositions(position, (x, y, pos) => {
    if (getDataOf(pos).isWinning) {
      ret(x, y, pos)
    }
  })
}

// Size of the current search space
let height = 3, width = 8

// Copied from my C++ code
function getPositionsWithNTiles(n, p, ret, y=height, h=width) {
  let dMin = Math.max(Math.floor(n / y), 0)
  let dMax = Math.min(n, h)

  for (let d = dMin; d <= dMax; ++d) {
    p[height - y] = d

    if (y === 1 || d === n) {
      ret(p)
    } else {
      getPositionsWithNTiles(n - d, p, ret, y-1, d)
    }
  }

  p[height - y] = 0
}

function getPositions (ret, maxWidth = width, maxHeight = height) {
  height = maxHeight
  width = maxWidth

  const positionStore = []
  for (let n = 1; n <= maxWidth * maxHeight; ++n) {
    getPositionsWithNTiles(n, positionStore, ret)
  }
}

function constructPositionMap (maxWidth=8, maxHeight=3) {
  setDataOf([], { isWinning: true, dte: 0 })

  getPositions(pos => {
    let isWinning = false
    let maxWinningDTE = 0
    let minLosingDTE = Infinity

    let winningCuts = 0
    let losingCuts = 0

    getAllCuts(pos, (x, y) => {
      let cuttedPosition = cutPosition(pos, x, y)
      let positionData = getDataOf(cuttedPosition)

      if (!positionData.isWinning) {
        isWinning = true
        minLosingDTE = Math.min(minLosingDTE, positionData.dte)
        winningCuts++
      } else {
        maxWinningDTE = Math.max(maxWinningDTE, positionData.dte)
        losingCuts++
      }
    })

    setDataOf(pos, {isWinning, dte: (isWinning ? minLosingDTE : maxWinningDTE) + 1, winningCuts, losingCuts })
  }, maxWidth, maxHeight)
}

// Helper class?
class GamePosition {
  constructor (arr) {
    this.arr = arr
  }

  data () {
    return getDataOf(this.arr)
  }

  toString () {
    return prettyPrintPosition(this.arr)
  }

  toCompactString () {
    return this.arr.join(', ')
  }

  isWinning () {
    return this.data().isWinning
  }

  dte () {
    return this.data().dte
  }

  getWinningCut () {
    let ret

    getAllWinningCuts(this.arr, (x, y, cut) => {
      ret = { x, y, cut }
    })

    return ret
  }

  getBestWinningCut () {
    let ret
    let shortestDelay = Infinity

    getAllWinningCuts(this.arr, (x, y, cut) => {
      cut = new GamePosition(cut)

      let dte = cut.dte()
      if (dte < shortestDelay) {
        shortestDelay = dte
        ret = { x, y, cut }
      }
    })

    return ret
  }

  getLongestDelayedCut () {
    let ret
    let longestDelay = -1

    getAllLosingCuts(this.arr, (x, y, cut) => {
      cut = new GamePosition(cut)

      let dte = cut.dte()
      if (dte > longestDelay) {
        longestDelay = dte
        ret = { x, y, cut }
      }
    })

    return ret
  }

  cutAt (x, y) {
    return new GamePosition(cutPosition(this.arr, x, y))
  }
}

function generateRectangle (rows, cols) {
  let arr = []

  for (let i = 0; i < rows; ++i) arr.push(cols)

  return arr
}

function stringifyDistribution (arr, startIndex=0) {
  // n in (0 ... arr.length): ... but we trim arr to the last nonzero element

  let index = arr.length
  while (index-- && !arr[index]);

  arr = arr.slice(0, index + 1)

  return `n in (${startIndex} ... ${arr.length + startIndex - 1}): ${arr.join(', ')}`
}

function analysis () {
  cow = ""

  let totalPositions = 0
  let winningPositions = 0
  let losingPositions = 0
  let winningCuts = 0

  let tileCountDistribution = Array(width * height + 1).fill(0)
  let winningTileCountDistribution = Array(width * height + 1).fill(0)
  let losingTileCountDistribution = Array(width * height + 1).fill(0)
  let winningCutDistribution = Array(10).fill(0)
  let winningCutDistributions = Array(width * height + 1).fill(0).map(d => Array(10).fill(0))
  let maxDistancesToEndDistribution = Array(width * height + 1).fill(0)

  getPositions(pos => {
    pos = new GamePosition(pos)
    let sqCount = getPositionSquareCount(pos.arr)

    const data = pos.data()

    if (data.isWinning) {
      winningPositions++

      winningTileCountDistribution[sqCount]++
    } else {
      losingPositions++
      losingTileCountDistribution[sqCount]++
    }

    winningCuts += data.winningCuts
    winningCutDistribution[data.winningCuts]++
    winningCutDistributions[sqCount][data.winningCuts]++
    maxDistancesToEndDistribution[sqCount] = Math.max(maxDistancesToEndDistribution[sqCount], data.dte)

    tileCountDistribution[sqCount]++

    totalPositions++
  })

  output ("Total positions: ", totalPositions)
  output ("Winning positions: ", winningPositions)
  output ("Losing positions: ", losingPositions)

  output(`Positions with n tiles, `, stringifyDistribution(tileCountDistribution))
  output(`Winning positions with n tiles, `, stringifyDistribution(winningTileCountDistribution))
  output(`Losing positions with n tiles, `, stringifyDistribution(losingTileCountDistribution))

  output(`(# winning cuts)/(# winning positions): ${winningCuts / winningPositions}`)
  output(`# positions with n winning cuts, `, stringifyDistribution(winningCutDistribution))

  for (let k = 1; k < width * height + 1; ++k) {
    output(`# positions with ${k} tiles and n winning cuts, `, stringifyDistribution(winningCutDistributions[k]))
  }

  output(`Maximum distance to end among positions with n tiles, `, stringifyDistribution(maxDistancesToEndDistribution))

}

let cow = ""

function output (...args) {
  cow += args.join(' ') + '\n'
}

function playOptimalGame (position) {
  position = new GamePosition(position)

  output (`Beginning from position ${position.toCompactString()}`)
  let player = "Player 1"

  let i = 0

  while (true) {
    let { isWinning, dte } = position.data()
    if (dte === 0 ) return

    let cutX, cutY

    output (`Position ${position.toCompactString()} is ${isWinning ? "winning" : "losing"}, with max distance to game end of ${dte}.`)
    output (position.toString())
    if (isWinning) {
      let { cut, x, y } = position.getBestWinningCut()

      cutX = x
      cutY = y
    } else {
      let { cut, x, y } = position.getLongestDelayedCut()

      cutX = x
      cutY = y
    }

    output (`${player} cuts the position at (${cutX + 1}, ${cutY + 1}).`)

    position = position.cutAt(cutX, cutY)

    if (player === "Player 1") player = "Player 2"
    if (++i > 20) break
  }

  output (player + " wins.")
}

console.time("compute")
constructPositionMap(13, 13)
console.timeEnd("compute")

analysis()
