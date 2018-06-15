defmodule Luex.OpaqueData do
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
