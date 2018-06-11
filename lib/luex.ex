defmodule Luex do
  require Logger

  @on_load :load_nif

  @doc false
  def load_nif do
    nif_file = Path.join(:code.priv_dir(:luex), "luex_nif")

    case :erlang.load_nif(nif_file, 0) do
      :ok -> :ok
      {:error, {:reload, _}} -> :ok
      {:error, reason} -> exit(reason)
    end
  end

  def init, do: :erlang.nif_error("luex nif not loaded")
  def dostring(_l, _str), do: :erlang.nif_error("luex nif not loaded")
  def dofile(_l, _str), do: :erlang.nif_error("luex nif not loaded")

  def test() do
    {:ok, l} = Luex.init()
    lua = """
    i = 0
    wow = coroutine.wrap(function ()
      while true do
       i = i + 1
       coroutine.yield(i)
      end
    end)

    -- use
    return wow()
    """
    {:ok, {1.0}} = Luex.dostring(l, lua)
    for i <- 0..10000000 do
      {:ok, {val}} = Luex.dostring(l, "return wow()")
      {:ok, {}} = Luex.dostring(l, "collectgarbage()")
      val
    end
  end
end
