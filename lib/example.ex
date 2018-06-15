defmodule Luex.Example do
  use Luex.Server

  def start_link do
    Luex.Server.start_link(__MODULE__, [], [name: __MODULE__])
  end

  def linit(l, state) do
    l
    |> register_function(:add, 2)
    {:ok, l, state}
  end

  def handle_lcall(:add, [a, b], l, state) do
    {:return, {a+b}, l, state}
  end
end
