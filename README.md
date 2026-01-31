# WARNING BEFORE YOU CONTINUE READING THIS README AND/OR THE CODE AND INTERACTING WITH THIS PROJECT YOU MUST AGREE WITH THE LICENSE OR ELSE DO NOT CONTINUE AND LEAVE THIS PAGE

# Edan's Efficient Markup Language (EML)

> **The definitive structural skeleton for UI.** Masters complex nesting with a clean, efficient syntax.

**Efficient Markup Language (EML)** is a concise, developer-friendly markup language designed to replace verbose XML-like syntax. It simplifies the structure of HTML, PHP, XAML, FXML, and XML by using C-style brackets `{}`, reducing visual noise and improving readability without sacrificing power or flexibility.

## 🚀 Key Features

*   **Concise Syntax**: Replaces `<tag></tag>` with `tag { }`. No more hunting for closing tags.
*   **Bidirectional Conversion**: Seamlessly convert **EML ↔ HTML**, **EML ↔ PHP**, **EML ↔ XML**, **EML ↔ XAML**, and **EML ↔ FXML**.
*   **Whitespace Fidelity**: Smart formatting preserves indentation and structure where it matters (scripts, styles, code blocks).
*   **Embedded Languages**: First-class support for `script`, `style`, and `<?php ... ?>` blocks.
*   **High Performance**: Powered by **EMLC**, a blazing-fast C++ compiler.

## 📦 Usage

The **EMLC** (EMLC) CLI tool handles all conversions.

```bash
# General Usage
emlc <input> <output> [options]

# Examples
emlc index.eml index.html       # Compile EML to HTML
emlc site.eml index.php         # Compile EML to PHP
emlc view.eml view.xaml         # Compile EML to XAML

emlc index.html index.eml       # Decompile HTML back to EML
```

## 📝 Syntax Comparison



**EML**
```eml
html(lang="en") {
    head {
        meta(charset="UTF-8")
        meta(name="viewport", content="width=device-width, initial-scale=1.0")
        title { Website }
    }
    body {
        form (method="POST", action="/") {
            label (for="username") {Username}
            input(type="text", name="username", placeholder="Username")
            label (for="password") {Password}
            input(type="text", name="password", placeholder="Password")
            input(type="submit", name="signup", value="Sign Up")
        }
    }
}
```

**HTML/XML**
```html
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title> Website </title>
    </head>
    <body>
        <form method="POST" action="/">
            <label for="username">Username</label>
            <input type="text" name="username" placeholder="Username">
            <label for="password">Password</label>
            <input type="text" name="password" placeholder="Password">
            <input type="submit" name="signup" value="Sign Up">
        </form>
    </body>
</html>

```

## 🌍 Multi-Platform Examples

### PHP (Web)
**EML**
```eml
php {
    require('auth.php');
}
html {
    head {
        title { My PHP Site }
    }
    body {
        h1 { Welcome to my PHP Site }
    }
}
```
**PHP**
```php
<?php
    require('auth.php');
?>
<html>
    <head>
        <title>My PHP Site</title>
    </head>
    <body>
        <h1>Welcome to my PHP Site</h1>
    </body>
</html>
```

### XAML (WPF/WinUI)
**EML**
```eml
Grid (Background="White") {
    RowDefinition (Height="Auto")
    RowDefinition (Height="*")

    TextBlock (Text="Welcome to XAML" FontSize="24" FontWeight="Bold") { }
    Button (Content="Click Me" HorizontalAlignment="Center") { }
}
```
**XAML**
```xml
<Grid Background="White">
    <RowDefinition Height="Auto" />
    <RowDefinition Height="*" />

    <TextBlock Text="Welcome to XAML" FontSize="24" FontWeight="Bold" />
    <Button Content="Click Me" HorizontalAlignment="Center" />
</Grid>
```

### FXML (JavaFX)
**EML**
```eml
VBox (alignment="CENTER" spacing="20.0" xmlns:fx="http://javafx.com/fxml") {
    Label (text="Welcome to JavaFX") { }
    Button (text="Action!" onAction="#handleButtonAction") { }
}
```
**FXML**
```xml
<VBox alignment="CENTER" spacing="20.0" xmlns:fx="http://javafx.com/fxml">
    <Label text="Welcome to JavaFX" />
    <Button text="Action!" onAction="#handleButtonAction" />
</VBox>
```

### Android XML
**EML**
```eml
LinearLayout (
    xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:orientation="vertical"
) {
    TextView (
        android:layout_width="wrap_content"
        android:layout_height="wrap_content"
        android:text="Hello Android!"
    ) { }

    Button (
        android:layout_width="wrap_content"
        android:layout_height="wrap_content"
        android:text="Click Me"
    ) { }
}
```
**XML**
```xml
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:orientation="vertical">

    <TextView
        android:layout_width="wrap_content"
        android:layout_height="wrap_content"
        android:text="Hello Android!" />

    <Button
        android:layout_width="wrap_content"
        android:layout_height="wrap_content"
        android:text="Click Me" />
</LinearLayout>
```
```

## 🛠️ Building

Requirements: Visual Studio 2022 (or newer) with C++ workload.

## 📄 License

Copyright (c) 2025 Edanick.
