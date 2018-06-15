defmodule Luex.Server do
  @type l :: Luex.l
  @type state :: term

  @type lfunction_name :: atom()
  @type lfunction_args :: any() # fixme
  @type lfunction_return :: tuple()

  @callback linit(l, any) :: {:ok, l, state}
  @callback handle_lcall(lfunction_name, lfunction_args, l, state) :: {:reply, lfunction_return, l, state}

  defmodule Priv do
    defstruct [:state, :l]
  end

  defmacro __using__(_) do
    quote do
      @behaviour Elixir.Luex.Server
      use GenServer
      alias Elixir.Luex.Server.Priv
      import Elixir.Luex

      # GenServer init
      def init(args) do
        l = Luex.new()
        case apply(__MODULE__, :linit, [l, args]) do
          {:ok, l, state} -> {:ok, struct(Priv, [state: state, l: l])}
        end
      end



    end
  end

  def start_link(module, args, opts \\ []) do
    GenServer.start_link(module, args, opts)
  end
end
