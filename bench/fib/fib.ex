defmodule Fib do
  def fib(0) do 0 end
  def fib(1) do 1 end
  def fib(n) do fib(n-1) + fib(n-2) end
end

timeit = fn({m, f, a}) ->
  start = :os.system_time(:seconds)
  res = apply(m, f, a)
  delta = :os.system_time(:seconds) - start
  {delta, res}
end

for n <- [10, 25, 35, 100] do
  IO.puts "Fib of #{n}"
  {time, value} = timeit.({Fib, :fib, [n]})
  IO.puts "    time: #{time} value: #{value}"
end
