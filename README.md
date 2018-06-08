# Luex

Run lua inside Elixir/Erlang

```elixir
{:ok, l} = Luex.init()
Luex.dostring(l, "while(true) do end")
```

and that's it. Now the vm is stuck forever. Good luck.

## Installation

If [available in Hex](https://hex.pm/docs/publish), the package can be installed
by adding `luex` to your list of dependencies in `mix.exs`:

```elixir
def deps do
  [
    {:luex, "~> 0.1.0"}
  ]
end
```

Documentation can be generated with [ExDoc](https://github.com/elixir-lang/ex_doc)
and published on [HexDocs](https://hexdocs.pm). Once published, the docs can
be found at [https://hexdocs.pm/luex](https://hexdocs.pm/luex).
