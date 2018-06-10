defmodule Luex.Shell.Evaluator do
  @moduledoc """
  The evaluator is responsible for managing the shell port and executing
  commands against it.
  """

  def init(command, server, leader, _opts) do
    old_leader = Process.group_leader
    Process.group_leader(self(), leader)

    command == :ack && :proc_lib.init_ack(self())
    {:ok, l} = Luex.init()
    state = %{l: l}

    try do
      loop(server, state)
    after
      Process.group_leader(self(), old_leader)
    end
  end

  defp loop(server, state) do
    receive do
      # {^port, {:data, data}} ->
      #   IO.puts("#{data}")
      #   loop(server, state)
      # {^port, {:exit_status, status}} ->
      #   IO.puts("Interactive shell port exited with status #{status}")
      #   :ok
      {:eval, ^server, command, shell_state} ->
        case Luex.dostring(state.l, command) do
          {:ok, res} -> IO.inspect(res)
          {:error, reason} -> IO.puts reason
        end

        new_shell_state = %{shell_state | counter: shell_state.counter + 1}
        send(server, {:evaled, self(), new_shell_state})
        loop(server, state)
      other ->
        IO.inspect(other, label: "Unknown message received by lua evaluator")
        loop(server, state)
    end
  end
end
