# Luex

Run lua inside Elixir/Erlang

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

## Example Usage

```elixir
{:ok, l} = Luex.init()
Luex.dostring(l, "while(true) do end")
```
and that's it. Now the vm is stuck forever. Good luck.

```elixir
Luex.dostring(l, "return 1 + 1")
{:ok, {2.0}}
```

```elixir
Luex.dostring(l, "whoops()")
{:error,
 "[string \"whoops()\r...\"]:1: attempt to call a nil value (global 'whoops')"}
```

```elixir
Luex.dostring(l, "return 1, 2, 3")
{:ok, {1.0, 2.0, 3.0}}
```

```elixir
Luex.dostring(l, "return {a = 1}")
{:ok, {%{"a" => 1.0}}}
```

```elixir
Luex.dostring(l, "return 'abcd'")
{:ok, {"abcd"}}
```

```elixir
Luex.dostring(l, "return 'cool string!', 0")
{:ok, {"cool string!", 0.0}}
```

```elixir
Luex.dostring(l, "return \"cool string!\", 1, nil, true, false, {a = 123, b = {c = 1}}")
{:ok,
 {"cool string!", 1.0, nil, true, false, %{"a" => 123.0, "b" => %{"c" => 1.0}}}}
```
