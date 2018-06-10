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
end
