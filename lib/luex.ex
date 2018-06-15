defmodule Luex do
  @moduledoc """
  Lua scripting interface from Elixir/Erlang!
  """

  @typedoc """
  Lua instance. This is a value stored inside the NIF.
  It should be opaque in it's usage.
  """
  @opaque l :: reference()

  @typedoc """
  Value returned from lua.
  """
  @type lua_return :: tuple()

  @doc """
  Initialize a new instance of Lua.
  """
  @spec new() :: {:ok, l} | {:error, binary}
  def new, do: :erlang.nif_error("luex nif not loaded")

  @spec dostring(l, binary) :: {:ok, lua_return} | {:error, binary}
  def dostring(_l, _str), do: :erlang.nif_error("luex nif not loaded")

  @spec dofile(l, Path.t()) :: {:ok, lua_return} | {:error, binary}
  def dofile(_l, _str), do: :erlang.nif_error("luex nif not loaded")

  def register_function(_l, _fun, _arity), do: :erlang.nif_error("luex nif not loaded")

  def into_mailbox(_, _data), do: :erlang.nif_error("luex nif not loaded")

  @on_load :load_nif
  @doc false
  def load_nif do
    nif_file = Path.join(:code.priv_dir(:luex), "luex_nif") |> to_charlist()

    case :erlang.load_nif(nif_file, 0) do
      :ok -> :ok
      {:error, {:reload, _}} -> :ok
      {:error, reason} -> {:error, :load_failed, reason}
    end
  end
end
