defmodule Luex do
  @moduledoc """
  Lua scripting interface from Elixir/Erlang!
  """

  defmodule OpaqueData do
    @moduledoc "Wrapper for opaque data that can't be viewed or modified."
    defstruct [:ptr, :name, :type]

    @typedoc "Opaque data that gets created in the NIF."
    @opaque t :: %__MODULE__{name: binary, ptr: integer, type: integer}

    @opaque l_table :: t
    @opaque l_function :: t
    @opaque l_userdata :: t
    @opaque l_thread :: t
    @opaque l_lightuserdata :: t

    defimpl Inspect, for: __MODULE__ do
      def inspect(%{ptr: ptr, name: name}, _) do
        formatted = :io_lib.format("0x~.16.0b", [ptr]) |> to_string
        "#<#{name} #{formatted}>"
      end
    end
  end

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
  @spec init() :: {:ok, l} | {:error, binary}
  def init, do: :erlang.nif_error("luex nif not loaded")

  @spec dostring(l, binary) :: {:ok, lua_return} | {:error, binary}
  def dostring(_l, _str), do: :erlang.nif_error("luex nif not loaded")

  @spec dofile(l, Path.t()) :: {:ok, lua_return} | {:error, binary}
  def dofile(_l, _str), do: :erlang.nif_error("luex nif not loaded")

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
