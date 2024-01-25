TOP=$(top_srcdir)

include $(TOP)/Make.config

# Keep all intermediate files always.
.SECONDARY:

.stamp-download-dotnet-packages: download-packages.csproj $(TOP)/Make.config
	$(Q_GEN) $(DOTNET) \
		build \
		$< \
		/p:PackageRuntimeIdentifiers="$(DOTNET_RUNTIME_IDENTIFIERS)" \
		/p:PackageRuntimeIdentifiersCoreCLR="$(DOTNET_CORECLR_RUNTIME_IDENTIFIERS)" \
		/p:CustomDotNetVersion="$(DOTNET_VERSION)" \
		/p:MonoToolChainManifestVersionBand="$(MONO_TOOLCHAIN_MANIFEST_VERSION_BAND)" \
		/p:EmscriptenManifestVersionBand="$(EMSCRIPTEN_MANIFEST_VERSION_BAND)" \
		/bl \
		$(DOTNET_BUILD_VERBOSITY)
	$(Q) touch $@

# dotnet build download-packages.csproj /p:PackageRuntimeIdentifiers="tvos-arm64 tvossimulator-x64 tvossimulator-arm64 maccatalyst-x64 maccatalyst-arm64 osx-x64 osx-arm64" /p:PackageRuntimeIdentifiersCoreCLR="osx-x64 osx-arm64" /p:CustomDotNetVersion="8.0.1" /p:MonoToolChainManifestVersionBand="8.0.100" /p:EmscriptenManifestVersionBand="8.0.100" /p:TrackingDotNetRuntimeSeparately="" /bl

DOTNET_DOWNLOADS = \
	.stamp-download-dotnet-packages

dotnet: $(DOTNET_DOWNLOADS)
all-local:: $(DOTNET_DOWNLOADS)

clean-local::
	$(Q) rm -Rf $(DOTNET_DOWNLOADS)

