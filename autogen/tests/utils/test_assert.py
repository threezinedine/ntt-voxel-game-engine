def AssertBindingResult(expect: str, result: str) -> None:
    assert ClearText(expect) == ClearText(
        result
    ), f"""Expect:
```
{ClearText(expect)}
```
But receive
```
{ClearText(result)}
```
    """


def ClearText(content: str) -> str:
    """
    Used for reformatting the result so that strips all content.
    """
    for i in range(len(content)):
        if content[i] not in [" ", "\n"]:
            content = content[i:]
            break

    for i in range(len(content) - 1, 0, -1):
        if content[i] not in [" ", "\n"]:
            content = content[: i + 1]
            break

    content = content.replace("\t", " ")
    content = content.replace("\n", " ")
    content = content.replace(" ", "")

    return content
