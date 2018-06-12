defmodule Fib do
  def fib(0) do 0 end
  def fib(1) do 1 end
  def fib(n) do fib(n-1) + fib(n-2) end

  def fib2(n, a \\ 0, b \\ 1)
  def fib2(0, a, _b) do a end
  def fib2(n, a, b) do fib2(n - 1, b, a + b) end
end

timeit = fn({m, f, a}) ->
  start = :os.system_time(:seconds)
  res = apply(m, f, a)
  delta = :os.system_time(:seconds) - start
  {delta, res}
end

for n <- [10, 25, 35, 100, 100000, 150000, 150500, 1505000] do
  IO.puts "Fib of #{n}"
  {time, value} = timeit.({Fib, :fib2, [n]})
  IO.puts "    time: #{time}"
end
