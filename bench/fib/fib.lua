-- Author: Michael-Keith Bernard
-- Date: May 22, 2012
-- Description: Various implementations of the Fibonacci sequence in Lua. Lua
-- has native support for tail-call elimination which is why `tail_call` and
-- `continuation` run in near constant time. For sufficiently large numbers of n
-- you can start to see linear performace characteristics (particularly for the
-- `continuation` implementation), but ultimately the `tail_call` implementation
-- is an order of magnitude faster than iteration even for values of n as small
-- as 500k.

-- Tail-optimized recursive
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

for _, n in ipairs({10, 25, 35, 100, 1000, 5000, 100000, 1000000, 5000000}) do
  print("Fib of "..n)
  print(string.format("  %s, time: %s", timeit(tail_call, n)))
end
