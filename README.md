capybara-chromium
=================

A [capybara](https://github.com/jnicklas/capybara) driver that uses [Chromium Embedded](https://bitbucket.org/chromiumembedded/cef/).

CEF Dependency and Installation Issues
-------------------------------------

capybara-chromium depends on Chromium Embedded, a framework for embedding
Chromium-based browsers in other applications. You'll need to download the CEF
libraries to build and install the gem.

CEF binaries can be downloaded from the [CEF Builds](https://cefbuilds.com/)
site. capybara-chromium is currently compatible with branch 2526.

capybara-chromium will look in the Release directory of this repository for its
dependencies. Copy the contents of the Release and Resources directories from
the archive into the Release directory, and then run make.

License
-------

capybara-chromium is Copyright (c) 2016 Matthew Horan. It is free software, and
may be redistributed under the terms specified in the [LICENSE] file.

[LICENSE]: /LICENSE

About
-----

The capybara-chromium driver is maintained by Matthew Horan.
