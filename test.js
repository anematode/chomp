
const buffer = new ArrayBuffer(8)
const arr = new Float64Array(buffer)
const int32arr = new Uint32Array(buffer)

function getFloatFromParts(a, b) {
  int32arr[0] = a
  int32arr[1] = b

  return arr[0]
}

let seed = 0

// Credit to https://stackoverflow.com/a/52171480/13458117. This is a 53-bit hash using floats, meaning it can
function hashPosition (position) {
  let h1 = 0xdeadbeef ^ seed, h2 = 0x41c6ce57 ^ seed;

  for (let i = 0, value; i < position.length; i++) {
    value = position[i]
    h1 = Math.imul(h1 ^ value, 2654435761);
    h2 = Math.imul(h2 ^ value, 1597334677);
  }

  h1 = Math.imul(h1 ^ (h1>>>16), 2246822507) ^ Math.imul(h2 ^ (h2>>>13), 3266489909);
  h2 = Math.imul(h2 ^ (h2>>>16), 2246822507) ^ Math.imul(h1 ^ (h1>>>13), 3266489909);
  return 4294967296 * (2097151 & h2) + (h1>>>0);
}

let nan1 = getFloatFromParts(0, 2146959360)
let nan2 = getFloatFromParts(0, 2146959361)

let map = new Map()

map.set(nan1, "hi")
map.set(nan2, "bye")

console.log(Object.is(nan1, nan2))


