-- https://gist.githubusercontent.com/SegFaultAX/2772595/raw/095fd6107259e62eac107e5aa94bd6c7c6508101/fib.lua
function tail_call(n)
  local function inner(m, a, b)
    if m == 0 then
      return a
    end
    return inner(m-1, b, a+b)
  end
  return inner(n, 0, 1)
end

function timeit(f, ...)
  local start = os.time()
  local res = { f(...) }
  local delta = os.time() - start
  return delta, unpack(res)
end

for _, n in ipairs({10, 25, 35, 100}) do
  print("Fib of "..n)
  print(string.format("    time: %s value: %s", timeit(tail_call, n)))
end
