defmodule LuexTest do
  use ExUnit.Case
  doctest Luex

  test "greets the world" do
    assert Luex.hello() == :world
  end
end
