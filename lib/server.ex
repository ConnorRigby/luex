defmodule Luex.Server do
  @type l :: Luex.l
  @type state :: term

  @type lfunction_name :: atom()
  @type lfunction_args :: any() # fixme
  @type lfunction_return :: tuple()

  @callback linit(l, any) :: {:ok, l, state}
  @callback handle_lcall(lfunction_name, lfunction_args, l, state) :: {:reply, lfunction_return, l, state}

  defmodule Priv do
    defstruct [:state, :l, :req]
  end

  defmacro __using__(_) do
    quote do
      @behaviour Elixir.Luex.Server
      use GenServer
      alias Elixir.Luex.Server.Priv
      import Elixir.Luex

      # GenServer init
      def init(args) do
        {:ok, l} = Luex.new()
        Process.flag(:trap_exit, true)
        case apply(__MODULE__, :linit, [l, args]) do
          {:ok, l, state} -> {:ok, struct(Priv, [state: state, l: l])}
        end
      end

      def handle_info({:EXIT, _, res}, priv) do
        GenServer.reply(priv.req, res)
        {:noreply, %{priv | req: nil}}
      end

      def handle_info({fun, args_tup}, priv) do
        case handle_lcall(String.to_atom(fun), Tuple.to_list(args_tup), priv.l, priv.state) do
          {:return, data, l, new_state} when is_tuple(data) ->
            into_mailbox(l, data)
            {:noreply, %{priv | state: new_state, l: l}}
        end
      end

      def handle_call({:dostring, str}, req, priv) do
        Process.link(spawn fn() ->
          dostring(priv.l, str)
          |> exit()
        end)
        {:noreply, %{priv | req: req}}
      end

    end
  end

  def start_link(module, args, opts \\ []) do
    GenServer.start_link(module, args, opts)
  end

  def dostring(pid, str, timeout \\ 5000) do
    GenServer.call(pid, {:dostring, str}, timeout)
  end
end
