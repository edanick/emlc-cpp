# WARNING BEFORE YOU CONTINUE READING THIS README AND/OR THE CODE AND INTERACTING WITH THIS PROJECT YOU MUST AGREE WITH THE LISCENSE OR ELSE DO NOT CONTINUE AND LEAVE THIS PAGE

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

**HTML/XML**
```html
<div class="container">
    <h1>Hello World</h1>
    <p>Welcome to EML.</p>
    <a href="https://example.com">Click Me</a>
</div>
```

**EML**
```eml
div (class="container") {
    h1 { Hello World }
    p { Welcome to EML. }
    a (href="https://example.com") { Click Me }
}

## 🌍 Multi-Platform Examples

### PHP (Web)
**EML**
```eml
php {
    require('auth.php');
}
html {
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
