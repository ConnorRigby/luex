defmodule Fib do
  def fib(0) do 0 end
  def fib(1) do 1 end
  def fib(n) do fib(n-1) + fib(n-2) end
end

for n <- [10, 25, 35, 100, 1000, 5000, 100000, 1000000, 5000000] do
  IO.puts "Fib of #{n}"
  {time, value} = :timer.tc(fn() ->
    Fib.fib(n)
  end)
  IO.puts "  #{value} time: #{time}"
end
